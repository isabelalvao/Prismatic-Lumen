#include "visual_uniforms.hpp"
#include <algorithm>
namespace fantasynt {
VisualUniforms visual_uniforms(const VisualConfig& c,const ArtMotion& motion,double time,bool extension) {
    VisualUniforms v{};
    const auto set=[&](int row,double a,double b,double x,double y) { v[row]={static_cast<float>(a),static_cast<float>(b),static_cast<float>(x),static_cast<float>(y)}; };
    const auto tint=c[Setting::TintMix]+(c[Setting::ExtensionTint]-c[Setting::TintMix])*(extension?extension_colour_presence(time,c):0);
    const double fog_scale=extension?1-(1-c[Setting::RedFogMultiplier])*red_phase_presence(time,c):1;
    set(0,c[Setting::EmissionGain],tint,c[Setting::FogBase]*fog_scale/100,c[Setting::FogGain]*fog_scale/100);
    set(1,c[Setting::TintStart]*100,c[Setting::TintEnd]*100,c[Setting::TintNear],c[Setting::TintFar]);
    const auto body=c.colour(ConfigColour::Body),fog=c.colour(ConfigColour::Fog),floor=c.colour(ConfigColour::Floor);
    set(2,body.r,body.g,body.b,c[Setting::NormalAmbient]);
    set(3,fog.r,fog.g,fog.b,c[Setting::FogAtmosphere]);
    set(4,c[Setting::PulseIdle],c[Setting::PulseGain],c[Setting::PulseCutoff],c[Setting::DarkPatternEmission]);
    set(5,c[Setting::PointHeight]*100,c[Setting::PointEdge]*100,c[Setting::PointEmission],c[Setting::DarkPatternLines]);
    set(6,c[Setting::TracePeriod]*100,c[Setting::TraceSpeed]*100,c[Setting::TraceDirection],c[Setting::TraceEmission]);
    set(7,c[Setting::TracePulse],c[Setting::TraceLower],c[Setting::TraceUpper],c[Setting::TraceAA]);
    set(8,c[Setting::HexEmission],c[Setting::HexPulse],c[Setting::HexNormal],c[Setting::PoleEmission]);
    const auto reveal=std::clamp((time-c[Setting::DarkPatternStart])/c[Setting::DarkPatternReveal],0.0,1.0);
    set(9,c[Setting::PolePulse],c[Setting::TreeTip]*100,c[Setting::PoleTip]*100,reveal*reveal*(3-2*reveal));
    set(10,c[Setting::PyramidLines],c[Setting::PyramidWidthMin],c[Setting::PyramidWidthMax],c[Setting::PyramidSlope]);
    set(11,c[Setting::PyramidEmission],c[Setting::PyramidPulse],c[Setting::PyramidRevealSoft],c[Setting::PyramidOrder]);
    set(12,c[Setting::SkyLongitude],c[Setting::SkyLatitude],c[Setting::SkyRows],c[Setting::DarkPatternWidth]);
    set(13,c[Setting::SkyDensity]*c.family_density(ArtRole::Point),c[Setting::SkyInner],c[Setting::SkyOuter],c[Setting::SkyEmission]);
    set(14,c[Setting::BloomScale],c[Setting::Exposure],c[Setting::Vignette],c[Setting::ToneMapping]);
    set(15,c[Setting::SkyInitial],c[Setting::SkyGrowth],c[Setting::Gamma],c[Setting::VignetteScale]);
    set(16,c[Setting::ReflectionBase],c[Setting::ReflectionPools],c[Setting::ReflectionFresnel],c[Setting::FresnelPower]);
    set(17,c[Setting::FloorGrid]*100,c[Setting::GridIntensity],c[Setting::ReflectionDisturbance],c[Setting::RoughX]);
    set(18,c[Setting::RoughY],c[Setting::FloorFog]*fog_scale/100,c[Setting::FloorFogGain]*fog_scale/100,c[Setting::FloorAtmosphere]);
    set(19,floor.r,floor.g,floor.b,c[Setting::ReflectionEnabled]);
    const auto colour=c.family_colour(ArtRole::Point);
    set(20,colour.r,colour.g,colour.b,c.family_enabled(ArtRole::Point)?c.family_emission(ArtRole::Point):0);
    set(21,motion.points,motion.traces,motion.pyramids,motion.sky);
    return v;
}
}
