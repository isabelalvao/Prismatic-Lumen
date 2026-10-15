#include "light_pencil.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt::interaction {
namespace {
vr::Vector3 sub(vr::Vector3 a,vr::Vector3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
float length(vr::Vector3 a) { return std::hypot(a.x,a.y,a.z); }
float distance(vr::Vector3 a,vr::Vector3 b) { return length(sub(a,b)); }
float alignment(vr::Vector3 a,vr::Vector3 b) {
    const float divisor=length(a)*length(b);
    if(!std::isfinite(divisor)||divisor<=1e-7f) return -2;
    const float value=(a.x*b.x+a.y*b.y+a.z*b.z)/divisor;
    return std::isfinite(value)?std::clamp(value,-1.0f,1.0f):-2;
}
bool usable(const PoseSample& p) {
    const auto v=p.pose.position;
    return p.position_valid&&p.position_tracked&&std::isfinite(v.x)&&std::isfinite(v.y)&&std::isfinite(v.z);
}
enum class Pose { ready, uncertain, released };
Pose classify(const HandSample& h,bool holding) {
    const auto p=[&](int i) { return h.joints[i].pose.position; };
    const float size=distance(p(1),p(7));
    if(size<0.035f||size>0.16f) return Pose::released;
    const float scale=std::clamp(size/0.084f,0.65f,1.4f);
    const float index=alignment(sub(p(8),p(7)),sub(p(10),p(8)));
    const float reach=distance(p(10),p(0))-distance(p(7),p(0));
    if(index<0.35f||reach<0.015f*scale) return Pose::released;
    bool uncertain=index<(holding?0.35f:0.60f)||reach<(holding?0.015f:0.025f)*scale;
    for(const int root:{12,17,22}) {
        if(!usable(h.joints[root])||!usable(h.joints[root+1])||!usable(h.joints[root+3])) {
            uncertain=true; continue;
        }
        const auto a=sub(p(root+1),p(root)),b=sub(p(root+3),p(root+1));
        const float bend=alignment(a,b);
        const float proximal=length(a);
        if(bend<-1||proximal<0.008f) { uncertain=true; continue; }
        const float extension=distance(p(root+3),p(root))/proximal;
        // Outro dedo claramente esticado encerra já; flexão parcial admite a mão natural.
        if(bend>0.85f&&extension>1.8f) return Pose::released;
        if(bend>(holding?0.82f:0.70f)&&extension>(holding?1.8f:1.65f)) uncertain=true;
    }
    if(!usable(h.joints[5])) uncertain=true;
    else {
        const float thumb=distance(p(5),p(0))/scale;
        const float spread=distance(p(5),p(7))/scale;
        if(thumb>0.105f||spread>0.095f) return Pose::released;
        if(thumb>(holding?0.10f:0.09f)||spread>(holding?0.09f:0.08f)) uncertain=true;
    }
    return uncertain?Pose::uncertain:Pose::ready;
}
}
bool pencil_hand_usable(const HandSample& h) {
    if(!h.active||h.locate_result<0) return false;
    // Só a base e o indicador são indispensáveis à posição da escrita. A incerteza
    // nos dedos recolhidos é tratada separadamente, nunca como confirmação da pose.
    for(const int i:{0,1,7,8,10})
        if(!usable(h.joints[i])) return false;
    return true;
}
bool index_only_pose(const HandSample& h) {
    if(!pencil_hand_usable(h)) return false;
    return classify(h,false)==Pose::ready;
}
std::vector<PencilEvent> LightPencil::update(const ObservationFrame& frame,std::array<bool,2> snap_blocked) {
    std::vector<PencilEvent> events;
    const auto emit=[&](std::size_t hand,std::string_view event,std::string_view reason) {
        events.push_back({hand,frame.xr_time_ns,frame.origin_epoch,event,reason});
    };
    const auto clear=[&](std::size_t hand,std::string_view reason) {
        auto& s=hands_[hand];
        if(s.writing||!s.points.empty()) emit(hand,"reset",reason);
        s={};
    };
    if(frame.frame_status!="located"||frame.xr_time_ns<=0||!usable(frame.head)||
       !frame.head.orientation_valid||!frame.head.orientation_tracked) {
        for(std::size_t hand=0;hand<2;++hand) clear(hand,"xr_unusable");
        previous_time_.reset(); return events;
    }
    const auto now=frame.xr_time_ns;
    if(previous_time_&&(now<=*previous_time_||now-*previous_time_>80'000'000||frame.origin_epoch!=epoch_)) {
        for(std::size_t hand=0;hand<2;++hand)
            clear(hand,frame.origin_epoch!=epoch_?"origin_changed":"clock_discontinuity");
    }
    const float dt=previous_time_&&now>*previous_time_?static_cast<float>((now-*previous_time_)/1e9):0;
    previous_time_=now; epoch_=frame.origin_epoch;
    for(std::size_t hand=0;hand<2;++hand) {
        auto& s=hands_[hand]; const auto& h=frame.hands[hand];
        if(!pencil_hand_usable(h)) { clear(hand,"hand_unusable"); continue; }
        std::erase_if(s.points,[&](const auto& p) { return now-p.xr_time_ns>=650'000'000; });
        if(snap_blocked[hand]) {
            if(s.writing) emit(hand,"ended","snap_priority");
            s.writing=false; s.candidate_since=0; s.last_confirmed=0; s.raw.reset(); s.filtered.reset();
            continue; // O rasto anterior desvanece; a próxima escrita terá outro segmento.
        }
        const auto pose=classify(h,s.writing);
        if(pose==Pose::ready) {
            if(!s.candidate_since) s.candidate_since=now;
            s.last_confirmed=now;
        }
        const bool expired=s.last_confirmed&&now-s.last_confirmed>(s.writing?120'000'000:80'000'000);
        if(pose==Pose::released||expired) {
            if(s.writing) emit(hand,"ended",expired?"pose_uncertain_timeout":"pose_released");
            s.writing=false; s.candidate_since=0; s.last_confirmed=0; s.raw.reset(); s.filtered.reset();
            continue; // O traço já escrito desvanece sem continuar a escrever.
        }
        if(!s.writing&&(pose!=Pose::ready||now-s.candidate_since<40'000'000)) continue;
        auto tip=h.joints[10].pose.position;
        const auto direction=sub(tip,h.joints[8].pose.position);
        const float magnitude=length(direction);
        if(!std::isfinite(magnitude)||magnitude<0.003f) { clear(hand,"degenerate_index"); continue; }
        const float offset=std::isfinite(h.joints[10].radius_m)&&h.joints[10].radius_m>0?
            std::clamp(h.joints[10].radius_m,0.004f,0.012f)+0.002f:0.008f;
        tip.x+=direction.x/magnitude*offset; tip.y+=direction.y/magnitude*offset; tip.z+=direction.z/magnitude*offset;
        if(s.raw&&distance(*s.raw,tip)>0.12f) { clear(hand,"tip_jump"); continue; }
        const float speed=s.raw&&dt>0?distance(*s.raw,tip)/dt:0;
        s.raw=tip;
        if(!s.writing) {
            s.writing=true; ++s.stroke; s.filtered=tip;
            emit(hand,"started","index_only");
        } else {
            // Mais amortecimento em repouso; menos atraso ao desenhar depressa.
            const float tau=0.008f+0.027f/(1+speed/0.2f);
            const float amount=1-std::exp(-dt/tau);
            auto& p=*s.filtered;
            p.x+=(tip.x-p.x)*amount; p.y+=(tip.y-p.y)*amount; p.z+=(tip.z-p.z)*amount;
        }
        const auto position=*s.filtered;
        if(s.points.empty()||s.points.back().stroke!=s.stroke||distance(s.points.back().position_m,position)>=0.002f) {
            if(s.points.size()>=pencil_max_points) s.points.erase(s.points.begin());
            s.points.push_back({position,now,s.stroke});
        } else s.points.back().xr_time_ns=now; // Ponta viva, sem acumular pontos quando está quieta.
    }
    return events;
}
}
