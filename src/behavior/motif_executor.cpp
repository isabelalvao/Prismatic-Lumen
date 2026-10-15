#include "motif_executor.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fantasynt::behavior {
using Json=nlohmann::json;
void MotifExecutor::emit(std::string_view event,std::size_t i,std::string_view reason) {
    const auto& p=agenda[i];
    // Um ancestral só é factual se tiver sido efetivamente aplicado nesta execução.
    bool ancestor_applied=false;
    for(std::size_t j=0;j<agenda.size();++j) if(agenda[j].id==p.ancestor&&applied_[j]) ancestor_applied=true;
    events_.push_back({{"type","motif_event"},{"event",event},{"plan_id",p.id},{"epoch",epoch_},
        {"scene_seconds",time_},{"planned_start",p.start},{"planned_end",p.start+p.duration},
        {"origin",p.origin=="autonomous_return"&&!ancestor_applied?"autonomous_excerpt":p.origin},
        {"planned_origin",p.origin},{"ancestor",ancestor_applied?Json(p.ancestor):Json(nullptr)},
        {"planned_ancestor",p.ancestor},{"reason",reason},{"curve","long_dip_smoothstep"},
        {"repertoire_version",repertoire_version},{"entry_seconds",ramp_seconds},{"exit_seconds",ramp_seconds},
        {"light_depth",dose_.light_attenuation},{"gain_depth",dose_.master_attenuation},{"peak",1},
        {"integral_seconds",p.duration-ramp_seconds},{"envelope",envelope(p,time_)},
        {"light_contribution",-dose_.light_attenuation*envelope(p,time_)},
        {"master_gain",1-dose_.master_attenuation*envelope(p,time_)}});
}
void MotifExecutor::restart(double t,bool paused,std::string_view reason) {
    if(!enabled_) return;
    if(!std::isfinite(t)||t<0) throw std::invalid_argument("Invalid motif time");
    if(initialized_) stop(reason);
    ++epoch_; time_=t; paused_=paused; initialized_=true; stopped_=false;
    events_.push_back({{"type","executor_event"},{"event","restarted"},{"scene_seconds",time_},
        {"epoch",epoch_},{"reason",reason},{"paused",paused_}});
    applied_.fill(false);
    for(std::size_t i=0;i<agenda.size();++i) {
        status_[i]=t>=agenda[i].start+agenda[i].duration?Status::Missed:Status::Pending;
        emit(status_[i]==Status::Missed?"expired":"scheduled",i,reason);
    }
    advance(t,paused);
}
void MotifExecutor::advance(double t,bool paused,std::string_view reason) {
    if(!enabled_||stopped_) return;
    if(!std::isfinite(t)||t<0) throw std::invalid_argument("Invalid motif time");
    if(!initialized_||t<time_) { restart(t,paused,"clock_restart"); return; }
    time_=t;
    if(paused!=paused_) {
        ++epoch_;
        events_.push_back({{"type","executor_event"},{"event",paused?"paused":"resumed"},
            {"scene_seconds",time_},{"epoch",epoch_},{"reason",reason}});
        for(std::size_t i=0;i<agenda.size();++i) {
            if(paused&&status_[i]==Status::Pending) { status_[i]=Status::Cancelled; emit("cancelled",i,reason); }
            else if(!paused&&status_[i]==Status::Cancelled&&t<agenda[i].start+agenda[i].duration) {
                status_[i]=Status::Pending; emit("scheduled",i,"resume_new_epoch");
            }
            else if(status_[i]==Status::Running) emit(paused?"suspended":"resumed",i,reason);
        }
        paused_=paused;
    }
    // Uma prévia fixa pode avaliar a curva em pausa, sem promover memória aplicada.
    if(paused_) return;
    for(std::size_t i=0;i<agenda.size();++i) {
        const auto& p=agenda[i];
        if(status_[i]==Status::Running&&t>=p.start+p.duration) {
            status_[i]=Status::Completed; emit("completed",i);
        }
        if(status_[i]==Status::Pending&&t>=p.start) {
            if(t>=p.start+p.duration) { status_[i]=Status::Missed; emit("expired",i,"missed_no_retroactive_start"); }
            else { status_[i]=Status::Running; applied_[i]=true; emit("started",i); }
        }
    }
}
void MotifExecutor::stop(std::string_view reason) {
    if(!enabled_||!initialized_||stopped_) return;
    for(std::size_t i=0;i<agenda.size();++i)
        if(status_[i]==Status::Running||status_[i]==Status::Pending) {
            status_[i]=Status::Cancelled; emit("cancelled",i,reason);
        }
    stopped_=true;
}
EffectiveState MotifExecutor::compose(const Parameters& directed) const {
    EffectiveState s{directed,directed};
    if(enabled_&&initialized_&&!stopped_) s.envelope=scheduled_envelope(time_);
    const double desired=directed.light_intensity-dose_.light_attenuation*s.envelope;
    s.effective.light_intensity=static_cast<float>(std::clamp(desired,0.0,1.0));
    s.light_contribution=s.effective.light_intensity-directed.light_intensity;
    s.clamped=desired<0||desired>1;
    s.gain=1-dose_.master_attenuation*s.envelope;
    return s;
}
Json MotifExecutor::state() const {
    Json archive=Json::array(),current=Json::array();
    for(std::size_t i=0;i<agenda.size();++i) if(applied_[i]) {
        archive.push_back(agenda[i].id);
        if(time_<=agenda[i].start+agenda[i].duration+90) current.push_back(agenda[i].id);
    }
    while(current.size()>2) current.erase(current.begin());
    const double e=enabled_&&!stopped_?scheduled_envelope(time_):0;
    const Opportunity* active=nullptr;
    for(const auto& p:agenda) if(enabled_&&initialized_&&!stopped_&&time_>=p.start&&time_<p.start+p.duration) active=&p;
    return {{"enabled",enabled_},{"epoch",epoch_},{"paused",paused_},{"stopped",stopped_},
        {"repertoire_version",repertoire_version},{"scene_seconds",time_},{"envelope",e},
        {"light_depth",dose_.light_attenuation},{"gain_depth",dose_.master_attenuation},
        {"plan_id",active&&!stopped_?Json(active->id):Json(nullptr)},
        {"phase",active?(time_<active->start+2?"entry":time_>=active->start+active->duration-2?"exit":"development"):"neutral"},
        {"current_motifs",current},{"applied_archive",archive},{"protected_window",protected_time(time_)},
        {"body_decisions",false},{"preview_only",paused_&&active&&archive.empty()}};
}
std::vector<Json> MotifExecutor::drain_events() { auto result=std::move(events_); events_.clear(); return result; }
}
