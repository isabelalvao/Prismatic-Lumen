#include "interaction/light_pencil.hpp"
#include "interaction/hand_snap.hpp"
#include "render/pencil_geometry.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <limits>

using namespace fantasynt;
using namespace fantasynt::interaction;
ObservationFrame pointing(int frame,float x=0) {
    ObservationFrame f; f.xr_time_ns=frame*11'111'111LL; f.frame_status="located";
    f.head.position_valid=f.head.position_tracked=f.head.orientation_valid=f.head.orientation_tracked=true;
    for(auto& h:f.hands) {
        h.active=true; h.status="located";
        for(auto& j:h.joints) { j.position_valid=j.position_tracked=true; j.radius_m=0.006f; j.pose.position={x,0,-0.45f}; }
        const auto set=[&](int i,float dx,float y) { h.joints[i].pose.position={x+dx,y,-0.45f}; };
        set(1,0,-0.04f); set(5,-0.025f,0.02f);
        set(7,-0.025f,0.04f); set(8,-0.025f,0.075f); set(9,-0.025f,0.10f); set(10,-0.025f,0.122f);
        set(12,0,0.04f); set(13,0,0.07f); set(15,0,0.018f);
        set(17,0.024f,0.035f); set(18,0.024f,0.062f); set(20,0.024f,0.015f);
        set(22,0.045f,0.025f); set(23,0.045f,0.052f); set(25,0.045f,0.005f);
    }
    return f;
}
ObservationFrame two_fingers(int n,float x=0) {
    auto f=pointing(n,x);
    for(auto& h:f.hands) h.joints[15].pose.position.y=0.12f;
    return f;
}
ObservationFrame snapping_index(int n,int contact_start=1,int release=9) {
    auto f=pointing(n);
    if(n>=contact_start) for(auto& h:f.hands) {
        h.joints[5].pose.position={0,0.08f,-0.45f};
        h.joints[15].pose.position.y=n<release?0.08f:0.018f;
        h.joints[14].pose.position.y=n<release?0.07f:0.035f;
    }
    return f;
}
int main() {
    auto f=pointing(1);
    CHECK(index_only_pose(f.hands[0])); CHECK(!index_only_pose(two_fingers(1).hands[0]));
    auto thumb=f.hands[0]; thumb.joints[5].pose.position.y=0.15f; CHECK(!index_only_pose(thumb));
    for(int tip:{20,25}) { auto extra=f.hands[0]; extra.joints[tip].pose.position.y=0.12f; CHECK(!index_only_pose(extra)); }
    auto folded=f.hands[0]; folded.joints[10].pose.position.y=0.02f; CHECK(!index_only_pose(folded));
    auto untracked=f.hands[0]; untracked.joints[20].position_tracked=false; CHECK(!index_only_pose(untracked));
    // O classificador é independente de translação e rotação rígidas da mão.
    auto rotated=f.hands[0];
    for(auto& j:rotated.joints) {
        const auto p=j.pose.position; j.pose.position={p.z+1,p.x+2,p.y+3};
    }
    CHECK(index_only_pose(rotated));
    LightPencil pencil;
    for(int n=1;n<=4;++n) { CHECK(pencil.update(pointing(n)).empty()); CHECK(!pencil.writing(0)); }
    const auto started=pencil.update(pointing(5)); CHECK(started.size()==2); CHECK(pencil.writing(0));
    for(int n=6;n<=35;++n) pencil.update(pointing(n,(n-7)*0.005f));
    CHECK(pencil.points(0).size()>10); CHECK(pencil.points(0).size()<=pencil_max_points);
    const auto now=pointing(35).xr_time_ns;
    const auto mesh=make_pencil_geometry(pencil.points(0),now);
    CHECK(!mesh.indices.empty()); CHECK(mesh.vertices.size()<65536);
    float min_fade=1,max_fade=0;
    for(const auto& v:mesh.vertices) {
        CHECK(std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z));
        CHECK(v.fade>=0&&v.fade<=1); min_fade=std::min(min_fade,v.fade); max_fade=std::max(max_fade,v.fade);
    }
    CHECK(max_fade>min_fade);
    for(auto i:mesh.indices) CHECK(i<mesh.vertices.size());
    const auto ended=pencil.update(two_fingers(36,0.14f));
    CHECK(ended.size()==2); CHECK(!pencil.writing(0)); CHECK(!pencil.points(0).empty());
    for(int n=37;n<100;++n) pencil.update(two_fingers(n,0.14f));
    CHECK(pencil.points(0).empty()); CHECK(make_pencil_geometry(pencil.points(0),pointing(100).xr_time_ns).vertices.empty());
    // Sem segmentos entre gestos separados, mesmo antes de o rasto anterior desaparecer.
    LightPencil separate;
    for(int n=1;n<=10;++n) separate.update(pointing(n,n*0.005f));
    separate.update(two_fingers(11));
    for(int n=12;n<=20;++n) separate.update(pointing(n,0.08f));
    CHECK(separate.points(0).front().stroke!=separate.points(0).back().stroke);
    for(int fault=0;fault<6;++fault) {
        LightPencil p; for(int n=1;n<=10;++n) p.update(pointing(n));
        auto bad=pointing(11);
        if(fault==0) bad.origin_epoch=1;
        if(fault==1) bad.xr_time_ns+=100'000'000;
        if(fault==2) bad.frame_status="not_rendering";
        if(fault==3) bad.xr_time_ns=1;
        if(fault==4) for(auto& h:bad.hands) h.joints[10].position_valid=false;
        if(fault==5) for(auto& h:bad.hands) h.joints[10].pose.position.x=std::numeric_limits<float>::quiet_NaN();
        p.update(bad); CHECK(!p.writing(0)); CHECK(p.points(0).empty());
    }
    LightPencil jump;
    for(int n=1;n<=10;++n) jump.update(pointing(n));
    jump.update(pointing(11,0.5f)); CHECK(jump.points(0).empty()); CHECK(!jump.writing(0));
    LightPencil steady;
    for(int n=1;n<=400;++n) steady.update(pointing(n));
    CHECK(steady.points(0).size()==1);
    // Mão natural: polegar relaxado e dedos parcialmente recolhidos, sem exigir um punho fechado.
    auto natural=pointing(1);
    for(auto& h:natural.hands) {
        h.joints[5].pose.position={-0.060f,0.055f,-0.45f};
        h.joints[15].pose.position={0.030f,0.090f,-0.45f};
        h.joints[9].position_tracked=false; // DIP redundante já não bloqueia a ponta válida.
    }
    CHECK(index_only_pose(natural.hands[0]));
    // Incerteza dos dedos ocultos não ativa por si só, nem cancela uma escrita confirmada por um frame.
    LightPencil unknown;
    for(int n=1;n<=30;++n) {
        auto frame=pointing(n); frame.hands[0].joints[20].position_tracked=false;
        unknown.update(frame); CHECK(!unknown.writing(0));
    }
    LightPencil tolerant;
    for(int n=1;n<=10;++n) tolerant.update(pointing(n));
    const auto stroke=tolerant.points(0).back().stroke;
    for(int n=11;n<=18;++n) {
        auto frame=pointing(n,(n-10)*0.004f);
        frame.hands[0].joints[20].position_tracked=false;
        CHECK(tolerant.update(frame).empty()); CHECK(tolerant.writing(0));
    }
    CHECK(tolerant.points(0).back().stroke==stroke);
    auto recovered=pointing(19,0.036f); tolerant.update(recovered); CHECK(tolerant.writing(0));
    for(int n=20;n<=35;++n) {
        auto frame=pointing(n,0.036f); frame.hands[0].joints[20].position_tracked=false;
        tolerant.update(frame);
    }
    CHECK(!tolerant.writing(0)); CHECK(!tolerant.points(0).empty());
    // Histerese na flexão do indicador; largar explicitamente a pose continua imediato.
    LightPencil hysteresis;
    for(int n=1;n<=10;++n) hysteresis.update(pointing(n));
    for(int n=11;n<=30;++n) {
        auto frame=pointing(n); frame.hands[0].joints[10].pose.position.x=0.06f;
        CHECK(!index_only_pose(frame.hands[0])); hysteresis.update(frame); CHECK(hysteresis.writing(0));
    }
    hysteresis.update(two_fingers(31)); CHECK(!hysteresis.writing(0));
    // O filtro reduz a oscilação da ponta parada, sem acumular um histórico ilimitado.
    LightPencil jitter;
    float largest=0;
    for(int n=1;n<=100;++n) {
        jitter.update(pointing(n,n%2?0.002f:-0.002f));
        if(n>20) largest=std::max(largest,std::abs(jitter.points(0).back().position_m.x+0.025f));
    }
    CHECK(largest<0.0015f);
    // Regressão real: indicador elegível durante a preparação/libertação do estalo.
    CHECK(index_only_pose(snapping_index(1).hands[0]));
    CHECK(index_only_pose(snapping_index(9).hands[0]));
    SnapDetector snap;
    LightPencil priority;
    std::int64_t snap_time=0;
    for(int n=1;n<=85;++n) {
        const auto frame=snapping_index(n);
        const auto snaps=snap.update(frame);
        if(!snaps.empty()) { CHECK(n==9); CHECK(snaps.size()==2); snap_time=frame.xr_time_ns; }
        const auto events=priority.update(frame,{snap.blocks_pencil(0),snap.blocks_pencil(1)});
        if(n<=58) { CHECK(!priority.writing(0)); CHECK(priority.points(0).empty()); }
        for(const auto& e:events) if(e.event=="started") CHECK(e.xr_time_ns>=snap_time+snap_cooldown_ns+40'000'000);
    }
    CHECK(snap_time>0); CHECK(priority.writing(0)); CHECK(priority.writing(1));
    // Interrompe um traço já ativo; só essa mão fica bloqueada e a retoma não une segmentos.
    SnapDetector independent_snap;
    LightPencil independent;
    std::uint64_t previous_stroke=0;
    for(int n=1;n<=90;++n) {
        auto frame=snapping_index(n,13,21); frame.hands[1]=pointing(n).hands[1];
        independent_snap.update(frame);
        const auto events=independent.update(frame,{independent_snap.blocks_pencil(0),independent_snap.blocks_pencil(1)});
        if(n==12) previous_stroke=independent.points(0).back().stroke;
        if(n==13) {
            CHECK(events.size()==1); CHECK(events[0].hand==0); CHECK(events[0].reason=="snap_priority");
            CHECK(!independent.writing(0)); CHECK(independent.writing(1));
        }
        if(n>=13&&n<=70) CHECK(!independent.writing(0));
        if(n==75) CHECK(independent.points(0).back().stroke!=previous_stroke);
        if(n>=5) CHECK(independent.writing(1));
    }
    CHECK(independent.writing(0));
    // Polegar pousado no médio já recolhido é uma pose normal do lápis, sem preparação de estalo.
    SnapDetector resting_snap;
    LightPencil resting;
    for(int n=1;n<=40;++n) {
        auto frame=pointing(n);
        for(auto& h:frame.hands) {
            h.joints[5].pose.position=h.joints[15].pose.position;
            h.joints[14].pose.position.y=0.030f;
        }
        CHECK(resting_snap.update(frame).empty());
        CHECK(!resting_snap.blocks_pencil(0)); CHECK(!resting_snap.blocks_pencil(1));
        resting.update(frame,{resting_snap.blocks_pencil(0),resting_snap.blocks_pencil(1)});
        if(n>=5) CHECK(resting.writing(0)&&resting.writing(1));
    }
    LightPencil continuous;
    for(int n=1;n<=2000;++n) {
        auto dense=pointing(n,n*0.004f); dense.xr_time_ns=n*1'000'000LL; continuous.update(dense);
    }
    CHECK(continuous.points(0).size()==pencil_max_points);
    // Conversão espacial e ausência de tubo através de uma mudança de stroke.
    const std::array<PencilPoint,2> pair{{{{0,0,-0.4f},1'000'000'000,1},{{0.02f,0,-0.4f},1'050'000'000,1}}};
    const auto connected=make_pencil_geometry(pair,1'060'000'000);
    auto cut=pair; cut[1].stroke=2;
    CHECK(connected.indices.size()>make_pencil_geometry(cut,1'060'000'000).indices.size());
    CHECK(make_pencil_geometry(pair,2'000'000'000).vertices.empty());
    return test_finish();
}
