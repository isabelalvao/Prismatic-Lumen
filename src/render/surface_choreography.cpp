#include "surface_choreography.hpp"
#include <algorithm>
#include <cmath>

namespace fantasynt {
Instance surface_fade_instance(const Instance& source,double opacity) {
    auto result=source;
    result.transform[7]=static_cast<float>(std::clamp(opacity,0.0,1.0));
    return result;
}
namespace {
double reveal(double elapsed, double duration) {
    const double progress = std::clamp(elapsed / std::max(duration, 0.0001), 0.0, 1.0);
    return progress * progress * (3 - 2 * progress);
}
double cone_delay(int segment, double seed, const SurfaceChoreographySettings& settings) {
    return std::clamp(segment, 0, 6) * settings.segment_interval
        + std::clamp(seed, 0.0, 1.0) * settings.per_cone_delay;
}
}

double spectral_piece_visibility(int piece,double seed,double time,double start,double interval,double reveal_seconds,double instance_delay) {
    return reveal(time-start-std::max(0,piece)*interval-std::clamp(seed,0.0,1.0)*instance_delay,reveal_seconds);
}

double beat_aligned_time(double nominal_time,std::span<const double> onsets) {
    const auto next=std::lower_bound(onsets.begin(),onsets.end(),nominal_time);
    // Um intervalo sem ataques não pode bloquear a construção ou esperar vários segundos.
    return next!=onsets.end()&&*next-nominal_time<=0.75?*next:nominal_time;
}

double rear_piece_visibility(double depth_cm,double radius_cm,const VisualConfig& c) {
    return 1-reveal(-depth_cm-std::max(0.0,radius_cm)-c[Setting::RetainBehind]*100,c[Setting::BehindFade]*100);
}

double retained_wrapped_depth(double depth_cm,double radius_cm,const VisualConfig& c) {
    const double behind=(c[Setting::RetainBehind]+c[Setting::BehindFade])*100+std::max(0.0,radius_cm);
    const double span=41600+behind;
    double depth=std::fmod(depth_cm+behind,span);
    if(depth<0) depth+=span;
    return depth-behind;
}

Instance directed_tunnel_instance(const Instance& source,double time,double camera,double alignment_camera,
    const VisualConfig& c,double speed_multiplier,double alignment_lead_m,double animation_clock) {
    auto item=source;
    auto& m=item.transform;
    if(source.variation==5) {
        const double piece_duration=c[Setting::RingAlignDuration]*0.5;
        auto progress=reveal(time-c[Setting::RingAlignStart]-source.phase*piece_duration,piece_duration);
        const double half_width=(c[Setting::CorridorWidth]*0.5+c[Setting::CorridorMargin])*100;
        const double height=c[Setting::EyeHeight]*100;
        const double half_height=std::max(std::abs(height-c[Setting::CorridorFloor]*100),std::abs(c[Setting::CorridorCeiling]*100-height));
        const double inner=std::max(c[Setting::RingInnerRadius]*100,std::hypot(half_width,half_height)+c[Setting::CorridorMargin]*100);
        const double scale=inner/0.796;
        const double spacing=c[Setting::RingSpacing]*100;
        const double lead=(alignment_lead_m>=0?alignment_lead_m:
            required_ring_lead(c[Setting::CameraSpeed]*std::max(1.0/0.3,speed_multiplier),c))*100;
        m[14]=static_cast<float>(alignment_camera+lead+std::round(source.phase*63)*spacing-camera);
        // Uma recarga de velocidade durante a montagem antecipa suavemente as peças próximas.
        if(time>=c[Setting::RingAlignStart]) progress=std::max(progress,1-reveal(m[14]-1000,3000));
        m[0]=static_cast<float>(m[0]+(scale-m[0])*progress);
        m[5]=static_cast<float>(m[5]+(scale-m[5])*progress);
        m[10]=static_cast<float>(m[10]+(90-m[10])*progress);
        m[12]*=static_cast<float>(1-progress);
        m[13]=static_cast<float>(m[13]+(height-m[13])*progress);
        // As peças formam-se à frente do utilizador antes de ele alcançar a primeira.
    } else {
        const double clock=animation_clock>=0?animation_clock:time*c[Setting::AnimationSpeed];
        const double seed=source.color_mix;
        const double speed=c[Setting::ArtifactSpeedMin]+(c[Setting::ArtifactSpeedMax]-c[Setting::ArtifactSpeedMin])*seed;
        const double amplitude=c[Setting::ArtifactDrift]*100;
        const double phase=seed*6.283185307179586;
        const double frequency=amplitude>0?speed*100/amplitude:0;
        const double angle=clock*c[Setting::ArtifactRotation]*0.0174532925199433*(0.35+seed*0.65)*(source.padding<0.5?-1:1);
        const float ca=static_cast<float>(std::cos(angle)),sa=static_cast<float>(std::sin(angle));
        // Rotação rígida da peça: as três escalas e as proporções mantêm-se.
        for(int col=0;col<3;++col) {
            const int k=col*4; const float x=m[k],y=m[k+1];
            m[k]=ca*x-sa*y; m[k+1]=sa*x+ca*y;
        }
        const double radius=2*(std::hypot(m[0],m[1],m[2])+std::hypot(m[4],m[5],m[6])+std::hypot(m[8],m[9],m[10]));
        m[14]=static_cast<float>(retained_wrapped_depth(m[14]-camera,radius,c));
        m[12]+=static_cast<float>(amplitude*std::sin(clock*frequency+phase));
        m[13]+=static_cast<float>(amplitude*0.6*std::sin(clock*frequency*0.71+phase*1.7));
        m[14]+=static_cast<float>(amplitude*0.4*std::sin(clock*frequency*1.13+phase*0.8));
        const auto progress=tunnel_departure_progress(source.padding,time,c[Setting::TunnelDepartureStart],c[Setting::TunnelDepartureDuration]);
        const double radial=std::hypot(m[12],m[13]-260);
        if(radial>0) {
            m[12]+=static_cast<float>(m[12]/radial*progress*c[Setting::TunnelDepartureDistance]*100);
            m[13]+=static_cast<float>((m[13]-260)/radial*progress*c[Setting::TunnelDepartureDistance]*100);
        }
        m[14]+=static_cast<float>(progress*c[Setting::TunnelDepartureDistance]*100*(0.25+source.phase*0.75));
        // Considera também os desvios e a rotação na folga do corredor.
        const double rx=std::abs(m[0])+std::abs(m[4])+std::abs(m[8]);
        const double ry=std::abs(m[1])+std::abs(m[5])+std::abs(m[9]);
        const double half=(c[Setting::CorridorWidth]*0.5+c[Setting::CorridorMargin])*100;
        if(m[13]+ry>c[Setting::CorridorFloor]*100&&m[13]-ry<c[Setting::CorridorCeiling]*100&&std::abs(m[12])<half+rx)
            m[12]=static_cast<float>((m[12]>=0?1:-1)*(half+rx));
    }
    return item;
}

double required_ring_lead(double maximum_speed_m_s,const VisualConfig& c) {
    const double speed=std::max(0.0,maximum_speed_m_s),duration=c[Setting::RingAlignDuration];
    return std::max({c[Setting::RingLead],speed*duration*0.5+10,
        speed*duration+10-63*c[Setting::RingSpacing]});
}

int cone_segment_index(double original_local_y) {
    return static_cast<int>(std::floor(std::clamp(original_local_y * 3.5 + 0.0001, 0.0, 6.0)));
}

DrillConeMotion drill_cone_motion(double seed,double time,const SurfaceChoreographySettings& c) {
    const auto start=c.cones_start+std::clamp(seed,0.0,1.0)*c.per_cone_delay;
    const auto surface=start+c.cone_emerge_seconds;
    return {reveal(time-start,c.cone_emerge_seconds),reveal(time-surface,c.cone_separate_seconds),
        surface,surface+c.cone_separate_seconds};
}
double cone_module_local_offset(int segment,const DrillConeMotion& motion) {
    // Fecha as folgas dos sete módulos na broca inteira; a ponta atravessa o piso primeiro.
    constexpr double closed_height=1.46,closed_gap=0.27*2/7;
    return -closed_height*(1-motion.rise)-std::clamp(segment,0,6)*closed_gap*(1-motion.split);
}
double tunnel_departure_progress(double seed,double time,double start,double duration) {
    const double delay=std::clamp(seed,0.0,1.0)*duration*0.15;
    return reveal(time-start-delay,duration-delay);
}

double cone_segment_visibility(int segment, double seed, double scene_time,
    const SurfaceChoreographySettings& settings) {
    return reveal(scene_time - settings.cones_start - cone_delay(segment, seed, settings),
        settings.segment_reveal_seconds);
}

double cone_segment_angle(int segment, double seed, double animation_clock,
    const SurfaceChoreographySettings& settings) {
    const double elapsed = std::max(0.0,
        animation_clock - settings.cone_rotation_start_clock - settings.segment_reveal_seconds
        - cone_delay(segment, seed, settings));
    return elapsed * settings.cone_rotation_speed * (std::clamp(segment, 0, 6) % 2 == 0 ? 1 : -1);
}

double previous_piece_visibility(double seed, double exit_progress) {
    return 1 - reveal(std::clamp(exit_progress, 0.0, 1.0) - std::clamp(seed, 0.0, 1.0) * 0.65, 0.35);
}

double tube_piece_visibility(double rank, double scene_time, double previous_exit_progress,
    const SurfaceChoreographySettings& settings) {
    if (previous_exit_progress < 1) return 0;
    const double reveal_seconds = std::max(settings.tube_piece_reveal_seconds, 0.0001);
    const double delay = std::clamp(rank, 0.0, 1.0)
        * std::max(0.0, settings.tube_build_seconds - reveal_seconds);
    return reveal(scene_time - settings.tube_start - delay, reveal_seconds);
}

double generic_piece_visibility(double seed, double scene_time, double family_start,
    const SurfaceChoreographySettings& settings) {
    const double reveal_seconds = std::max(settings.segment_reveal_seconds, 0.0001);
    const double delay = std::clamp(seed, 0.0, 1.0)
        * std::max(0.0, settings.generic_forms_reveal_seconds - reveal_seconds);
    return reveal(scene_time - family_start - delay, reveal_seconds);
}

}
