#include "hand_snap.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt::interaction {
namespace {
float distance(vr::Vector3 a,vr::Vector3 b) { return std::hypot(a.x-b.x,a.y-b.y,a.z-b.z); }
bool usable(const PoseSample& joint) {
    const auto p=joint.pose.position;
    return joint.position_valid&&joint.position_tracked&&std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
}
}
bool snap_hand_usable(const HandSample& h) {
    if(!h.active||h.locate_result<0) return false;
    for(const auto i:{0,1,5,12,14,15}) if(!usable(h.joints[i])) return false;
    return true;
}
void SnapDetector::reset() { hands_={}; previous_time_.reset(); }
bool SnapDetector::blocks_pencil(std::size_t hand) const {
    const auto& s=hands_[hand];
    return (s.pencil_reserved&&(s.contact_since||s.armed))||(previous_time_&&*previous_time_<s.cooldown_until);
}
std::vector<SnapEvent> SnapDetector::update(const ObservationFrame& frame) {
    std::vector<SnapEvent> result;
    const auto disarm=[](HandState& state) {
        const auto cooldown=state.cooldown_until;
        state={}; state.cooldown_until=cooldown;
    };
    if(frame.frame_status!="located"||frame.xr_time_ns<=0||!usable(frame.head)||
       !frame.head.orientation_valid||!frame.head.orientation_tracked) {
        for(auto& h:hands_) disarm(h);
        return result;
    }
    if(previous_time_) {
        if(frame.xr_time_ns<=*previous_time_) hands_={};
        else if(frame.origin_epoch!=epoch_||frame.xr_time_ns-*previous_time_>80'000'000)
            for(auto& h:hands_) disarm(h);
    }
    previous_time_=frame.xr_time_ns; epoch_=frame.origin_epoch;
    const auto now=frame.xr_time_ns;
    for(std::size_t i=0;i<hands_.size();++i) {
        auto& s=hands_[i]; const auto& h=frame.hands[i];
        if(!snap_hand_usable(h)) { disarm(s); continue; }
        if(now<s.cooldown_until) continue;
        const auto point=[&](int j) { return h.joints[j].pose.position; };
        const float size=distance(point(1),point(12));
        if(size<0.035f||size>0.16f) { s={}; continue; }
        const float scale=std::clamp(size/0.08f,0.65f,1.4f);
        const float contact=distance(point(5),point(15));
        const float tip=distance(point(0),point(15)),distal=distance(point(0),point(14));
        if(contact<=0.024f*scale) {
            if(!s.contact_since) s.contact_since=now;
            // Contacto junto à palma também ocorre ao apontar: só reserva se o médio
            // tem espaço para a recolha do estalo. A reserva mantém-se até abandonar o candidato.
            s.pencil_reserved=s.pencil_reserved||(tip>=0.040f*scale&&distal>=0.025f*scale);
            s.last_contact=now; s.tip_distance=tip; s.distal_distance=distal;
            if(now-s.contact_since>=45'000'000) s.armed=true;
            continue;
        }
        if(s.armed&&s.last_contact&&now-s.last_contact<=180'000'000) {
            const float fold=s.tip_distance-tip,distal_fold=s.distal_distance-distal;
            const float elapsed=static_cast<float>((now-s.last_contact)/1e9);
            const float speed=fold/elapsed;
            if(contact>=0.035f*scale&&fold>=0.018f*scale&&fold<=0.10f*scale&&
               distal_fold>=0.010f*scale&&speed>=0.30f*scale) {
                result.push_back({i,now,epoch_,point(15),fold,speed});
                s={}; s.cooldown_until=now+snap_cooldown_ns;
                continue;
            }
        } else { s={}; }
        if(!s.armed) s.contact_since=0;
    }
    return result;
}
}
