#include "visual_uniforms.hpp"
namespace fantasynt {
VisualUniforms visual_uniforms(const VisualConfig& c,const ArtMotion& motion) {
    VisualUniforms v{};
    const auto set=[&](int row,double a,double b,double x,double y) { v[row]={static_cast<float>(a),static_cast<float>(b),static_cast<float>(x),static_cast<float>(y)}; };
    set(0,c[Setting::EmissionGain],c[Setting::TintMix],c[Setting::FogBase]/100,c[Setting::FogGain]/100);
    set(1,c[Setting::TintStart]*100,c[Setting::TintEnd]*100,c[Setting::TintNear],c[Setting::TintFar]);
    const auto body=c.colour(ConfigColour::Body),fog=c.colour(ConfigColour::Fog),floor=c.colour(ConfigColour::Floor);
    set(2,body.r,body.g,body.b,c[Setting::NormalAmbient]);
    set(3,fog.r,fog.g,fog.b,c[Setting::FogAtmosphere]);
    set(4,c[Setting::PulseIdle],c[Setting::PulseGain],c[Setting::PulseCutoff],0);
    set(5,c[Setting::PointHeight]*100,c[Setting::PointEdge]*100,c[Setting::PointEmission],0);
    set(6,c[Setting::TracePeriod]*100,c[Setting::TraceSpeed]*100,c[Setting::TraceDirection],c[Setting::TraceEmission]);
    set(7,c[Setting::TracePulse],c[Setting::TraceLower],c[Setting::TraceUpper],c[Setting::TraceAA]);
    set(8,c[Setting::HexEmission],c[Setting::HexPulse],c[Setting::HexNormal],c[Setting::PoleEmission]);
    set(9,c[Setting::PolePulse],c[Setting::TreeTip]*100,c[Setting::PoleTip]*100,0);
    set(10,c[Setting::PyramidLines],c[Setting::PyramidWidthMin],c[Setting::PyramidWidthMax],c[Setting::PyramidSlope]);
    set(11,c[Setting::PyramidEmission],c[Setting::PyramidPulse],c[Setting::PyramidRevealSoft],c[Setting::PyramidOrder]);
    set(12,c[Setting::SkyLongitude],c[Setting::SkyLatitude],c[Setting::SkyRows],0);
    set(13,c[Setting::SkyDensity]*c.family_density(ArtRole::Point),c[Setting::SkyInner],c[Setting::SkyOuter],c[Setting::SkyEmission]);
    set(14,c[Setting::BloomScale],c[Setting::Exposure],c[Setting::Vignette],c[Setting::ToneMapping]);
    set(15,c[Setting::SkyInitial],c[Setting::SkyGrowth],c[Setting::Gamma],c[Setting::VignetteScale]);
    set(16,c[Setting::ReflectionBase],c[Setting::ReflectionPools],c[Setting::ReflectionFresnel],c[Setting::FresnelPower]);
    set(17,c[Setting::FloorGrid]*100,c[Setting::GridIntensity],c[Setting::ReflectionDisturbance],c[Setting::RoughX]);
    set(18,c[Setting::RoughY],c[Setting::FloorFog]/100,c[Setting::FloorFogGain]/100,c[Setting::FloorAtmosphere]);
    set(19,floor.r,floor.g,floor.b,c[Setting::ReflectionEnabled]);
    const auto colour=c.family_colour(ArtRole::Point);
    set(20,colour.r,colour.g,colour.b,c.family_enabled(ArtRole::Point)?c.family_emission(ArtRole::Point):0);
    set(21,motion.points,motion.traces,motion.pyramids,motion.sky);
    return v;
}
}
