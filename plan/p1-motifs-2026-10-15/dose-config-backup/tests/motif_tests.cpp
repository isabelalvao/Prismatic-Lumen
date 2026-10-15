#include "behavior/motif_executor.hpp"
#include "audio/master_envelope.hpp"
#include "render/art_direction.hpp"
#include "control/protocol.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <limits>
#include <vector>

using namespace fantasynt;
using namespace fantasynt::behavior;
int main() {
    Parameters base;
    MotifExecutor off(false);
    off.restart(54,false); off.advance(55,false);
    CHECK(parameters_to_json(off.compose(base).effective)==parameters_to_json(base));
    CHECK(off.compose(base).gain==1); CHECK(off.drain_events().empty());
    int initiatives=0;
    for(const auto& p:agenda) {
        initiatives+=p.origin=="autonomous";
        CHECK(p.duration>=6&&p.duration<=12);
        CHECK(p.start+p.duration<=neutral_deadline);
        CHECK(!protected_time(p.start+0.001));
        CHECK(envelope(p,p.start)==0); CHECK(envelope(p,p.start+p.duration)==0);
        CHECK_NEAR(envelope(p,p.start+2),1,1e-12);
        for(const double edge:{p.start,p.start+2,p.start+p.duration-2,p.start+p.duration}) {
            CHECK_NEAR(envelope(p,edge-1e-5),envelope(p,edge+1e-5),1e-4);
        }
        double integral=0,previous=0;
        for(double t=p.start;t<p.start+p.duration;t+=0.001) {
            const double e=envelope(p,t);
            CHECK(e>=0&&e<=1); CHECK(std::abs(e-previous)<0.00076);
            previous=e; integral+=e*0.001;
        }
        CHECK_NEAR(integral,p.duration-2,0.002);
    }
    CHECK(initiatives==3);
    for(double t=0;t<425;t+=0.01) {
        if(protected_time(t)) CHECK(scheduled_envelope(t)==0);
        CHECK(master_gain(t)>=0.88&&master_gain(t)<=1);
    }
    MotifExecutor first(true),replay(true);
    first.restart(0,false); replay.restart(0,false);
    CHECK(first.drain_events()==replay.drain_events());
    for(double t=0;t<425;t+=0.025) {
        first.advance(t,false); replay.advance(t,false);
        CHECK(first.drain_events()==replay.drain_events());
        CHECK(first.state()==replay.state());
        CHECK(first.state()["current_motifs"].size()<=2);
        const auto s=first.compose(base);
        CHECK(s.effective.light_intensity>=0.54f&&s.effective.light_intensity<=0.6f);
        CHECK_NEAR(s.gain,master_gain(t),1e-12);
        auto expected=parameters_to_json(base); expected["light_intensity"]=s.effective.light_intensity;
        CHECK(parameters_to_json(s.effective)==expected);
    }
    CHECK(first.state()["applied_archive"].size()==8);
    CHECK(first.state()["current_motifs"].size()<=2);
    MotifExecutor pause(true);
    pause.restart(0,false); pause.drain_events(); pause.advance(54,false); pause.drain_events();
    const auto before=pause.compose(base);
    pause.advance(54,true); const auto cancelled=pause.drain_events();
    CHECK(std::count_if(cancelled.begin(),cancelled.end(),[](auto& e){return e["event"]=="cancelled";})==7);
    CHECK_NEAR(pause.compose(base).gain,before.gain,1e-12);
    const auto epoch=pause.state()["epoch"];
    pause.advance(54,true); CHECK(pause.drain_events().empty());
    pause.advance(54,false); CHECK(pause.state()["epoch"]!=epoch);
    CHECK_NEAR(pause.compose(base).gain,before.gain,1e-12);
    pause.drain_events(); pause.advance(62,false); auto completed=pause.drain_events();
    CHECK(completed.size()==1&&completed[0]["event"]=="completed");
    CHECK(pause.compose(base).gain==1);
    pause.restart(0,false); auto restarted=pause.drain_events();
    CHECK(pause.state()["applied_archive"].empty()); CHECK(pause.compose(base).gain==1);
    CHECK(std::any_of(restarted.begin(),restarted.end(),[](auto& e){return e["event"]=="cancelled";}));
    pause.stop("test_exit"); CHECK(pause.compose(base).gain==1);
    MotifExecutor missed(true); missed.restart(100,false); auto missed_events=missed.drain_events();
    CHECK(missed.state()["applied_archive"].empty());
    CHECK(std::none_of(missed_events.begin(),missed_events.end(),[](auto& e){return e["event"]=="started";}));
    missed.advance(154,false); auto started=missed.drain_events();
    CHECK(started.back()["ancestor"].is_null());
    bool rejected=false;
    try { missed.advance(std::numeric_limits<double>::quiet_NaN(),false); }
    catch(const std::invalid_argument&) { rejected=true; }
    CHECK(rejected);
    // O fade e a composição artística mantêm-se fora do executor; a partir de 392 é identidade.
    VisualConfig config;
    for(double t:{392.0,408.1821315192744,416.0,424.1821315192744}) {
        MotifExecutor tail(true); tail.restart(t,false);
        const auto directed=directed_visual_parameters(base,t,config);
        CHECK(parameters_to_json(tail.compose(directed).effective)==parameters_to_json(directed));
        CHECK(tail.compose(directed).gain==1);
    }
    // Processamento PCM por blocos dá exatamente o mesmo resultado que o bloco completo.
    constexpr int rate=44100;
    std::vector<std::int16_t> source(14*rate*2);
    for(std::size_t i=0;i<source.size();++i) source[i]=i%2?32767:-32768;
    auto full=source,blocks=source;
    apply_master_envelope(full,rate,49*rate);
    constexpr std::size_t block_samples=1024;
    for(std::size_t i=0;i<blocks.size();i+=block_samples) {
        const auto n=std::min(block_samples,blocks.size()-i);
        apply_master_envelope(std::span(blocks).subspan(i,n),rate,49*rate+i/2);
    }
    CHECK(full==blocks); CHECK(full.size()==source.size());
    for(std::size_t i=0;i<full.size()/2;++i) {
        const double gain=master_gain(49.0+static_cast<double>(i)/rate);
        CHECK_NEAR(full[2*i],-32768*gain,0.500001);
        CHECK_NEAR(full[2*i+1],32767*gain,0.500001);
        if(i) CHECK(std::abs(full[2*i]-full[2*i-2])<=1);
    }
    auto final=source; apply_master_envelope(final,rate,392*rate); CHECK(final==source);
    return test_finish();
}
