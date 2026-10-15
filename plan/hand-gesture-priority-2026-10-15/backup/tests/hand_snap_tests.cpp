#include "interaction/hand_snap.hpp"
#include "render/snap_geometry.hpp"
#include "test_support.hpp"
#include <limits>
using namespace fantasynt;
using namespace fantasynt::interaction;

ObservationFrame frame(int n,bool release=false) {
    ObservationFrame f; f.frame_status="located"; f.xr_time_ns=n*11'111'111LL;
    f.head.position_valid=f.head.position_tracked=f.head.orientation_valid=f.head.orientation_tracked=true;
    for(auto& h:f.hands) {
        h.active=true; h.status="located";
        for(auto& j:h.joints) { j.position_valid=j.position_tracked=true; j.pose.position={0,0,-0.4f}; }
        h.joints[1].pose.position.y=-0.04f; h.joints[12].pose.position.y=0.04f;
        h.joints[5].pose.position.y=0.10f;
        h.joints[15].pose.position={0.005f,release?0.025f:0.10f,-0.4f};
        h.joints[14].pose.position.y=release?0.04f:0.08f;
    }
    return f;
}
void prime(SnapDetector& detector) { for(int n=1;n<=8;++n) CHECK(detector.update(frame(n)).empty()); }
int main() {
    SnapDetector snap; prime(snap);
    const auto events=snap.update(frame(9,true)); CHECK(events.size()==2);
    for(std::size_t hand=0;hand<events.size();++hand) {
        const auto& e=events[hand]; CHECK(e.hand==hand); CHECK(e.closing_m_s>0.3f);
        const auto mesh=make_snap_geometry(e,e.xr_time_ns+80'000'000);
        CHECK(mesh.vertices.size()==14*6); CHECK(mesh.indices.size()==14*24);
        for(const auto& v:mesh.vertices) CHECK(std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z));
        for(auto index:mesh.indices) CHECK(index<mesh.vertices.size());
        CHECK(make_snap_geometry(e,e.xr_time_ns-1).vertices.empty());
        CHECK(make_snap_geometry(e,e.xr_time_ns+450'000'000).vertices.empty());
        CHECK_NEAR(snap_particle_fade(e,e.xr_time_ns),1,1e-6);
        auto relocated=e; relocated.position_m.x+=1;
        const auto moved=make_snap_geometry(relocated,e.xr_time_ns+80'000'000);
        CHECK_NEAR(moved.vertices[0].x-mesh.vertices[0].x,100,1e-4);
    }
    auto lost_during_cooldown=frame(10); lost_during_cooldown.hands[0].active=false;
    CHECK(snap.update(lost_during_cooldown).empty());
    for(int n=11;n<=18;++n) CHECK(snap.update(frame(n)).empty());
    CHECK(snap.update(frame(19,true)).empty());
    for(int n=20;n<45;++n) CHECK(snap.update(frame(n,true)).empty());
    for(int n=61;n<=68;++n) CHECK(snap.update(frame(n)).empty());
    CHECK(snap.update(frame(69,true)).size()==2);
    // Pose mantida ou movimento rígido da mão inteira não disparam.
    SnapDetector still,moving;
    for(int n=1;n<=200;++n) {
        CHECK(still.update(frame(n)).empty());
        auto f=frame(n);
        for(auto& h:f.hands) for(auto& j:h.joints) j.pose.position.x+=n*0.01f;
        CHECK(moving.update(f).empty());
    }
    // Separar polegar sem dobrar o dedo médio não constitui o gesto.
    SnapDetector pinch; prime(pinch); auto separate=frame(9);
    for(auto& h:separate.hands) h.joints[5].pose.position.x+=0.06f;
    CHECK(pinch.update(separate).empty());
    SnapDetector short_contact;
    CHECK(short_contact.update(frame(1)).empty()); CHECK(short_contact.update(frame(2,true)).empty());
    // A articulação distal também tem de confirmar a dobra.
    SnapDetector tip_only; prime(tip_only); auto tip=frame(9,true);
    for(auto& h:tip.hands) h.joints[14].pose.position.y=0.08f;
    CHECK(tip_only.update(tip).empty());
    for(int fault=0;fault<7;++fault) {
        SnapDetector detector; prime(detector); auto f=frame(9,true);
        if(fault==0) f.origin_epoch=1;
        if(fault==1) f.xr_time_ns+=100'000'000;
        if(fault==2) f.frame_status="not_rendering";
        if(fault==3) f.xr_time_ns=1;
        if(fault==4) for(auto& h:f.hands) h.joints[15].position_tracked=false;
        if(fault==5) for(auto& h:f.hands) h.active=false;
        if(fault==6) for(auto& h:f.hands) h.joints[14].pose.position.x=std::numeric_limits<float>::quiet_NaN();
        CHECK(detector.update(f).empty());
    }
    // A perda de uma mão não invalida o gesto da outra.
    SnapDetector partial; prime(partial); auto one=frame(9,true); one.hands[0].active=false;
    const auto only=partial.update(one); CHECK(only.size()==1&&only[0].hand==1);
    // Dobra lenta não dispara; a velocidade é relativa à palma, não ao avanço virtual.
    SnapDetector slow; prime(slow);
    for(int n=9;n<50;++n) {
        auto f=frame(n); const float fold=std::min(0.075f,(n-8)*0.0015f);
        for(auto& h:f.hands) { h.joints[15].pose.position.y-=fold; h.joints[14].pose.position.y-=fold*0.53f; }
        CHECK(slow.update(f).empty());
    }
    return test_finish();
}
