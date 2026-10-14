#pragma once
#include "control/parameters.hpp"
#include <array>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>

namespace fantasynt {
enum class ArtRole;
enum class Setting {
AutoPlay,
VrScale,
HotReload,
PollInterval,
ParameterTransition,
CameraDelay,
CameraAcceleration,
CameraSpeed,
CameraStartZ,
EyeHeight,
EyeBack,
DesktopFov,
NearPlane,
FarPlane,
AnimationSpeed,
OpeningDuration,
PyramidStart,
PyramidReveal,
ForestStart,
GroundStart,
IntroEnd,
DomeEnd,
StageFade,
Handoff,
InitialPresence,
PresenceGrowth,
PointInitial,
PointFinal,
PointGrowth,
SkyInitial,
SkyGrowth,
CorridorWidth,
CorridorMargin,
CorridorFloor,
CorridorCeiling,
IntroSectors,
SectorSpacing,
WorldScale,
OffsetX,
OffsetY,
OffsetZ,
CullMultiplier,
PointSpeed,
PointHeight,
PointSize,
PointEdge,
PointEmission,
SkySpeed,
SkyLongitude,
SkyLatitude,
SkyRows,
SkyDensity,
SkyInner,
SkyOuter,
SkyEmission,
TraceSpeed,
TracePeriod,
TraceDirection,
TraceLower,
TraceUpper,
TraceAA,
TraceEmission,
TracePulse,
PyramidRotation,
PyramidAlternate,
PyramidLines,
PyramidWidthMin,
PyramidWidthMax,
PyramidSlope,
PyramidRevealSoft,
PyramidOrder,
PyramidEmission,
PyramidPulse,
PulseGain,
PulseIdle,
PulseCutoff,
PulseOffset,
BassLow,
BassHigh,
PulseAttack,
PulseDecay,
PulseTail,
PulseInterval,
PulseRelative,
PulseMinimum,
PulseBackground,
PulseReference,
PulseCompensation,
HexEmission,
HexPulse,
HexNormal,
PoleEmission,
PolePulse,
TreeTip,
PoleTip,
EmissionGain,
TintMix,
TintNear,
TintFar,
TintStart,
TintEnd,
NormalAmbient,
FogBase,
FogGain,
FogAtmosphere,
ReflectionEnabled,
ReflectionBase,
ReflectionPools,
ReflectionFresnel,
FresnelPower,
FloorGrid,
GridIntensity,
ReflectionDisturbance,
RoughX,
RoughY,
FloorFog,
FloorFogGain,
FloorAtmosphere,
BloomScale,
Exposure,
Vignette,
VignetteScale,
ToneMapping,
Gamma,
EnableDark,
DensityDark,
EmissionDark,
EnableHex,
DensityHex,
EmissionHex,
EnableTrace,
DensityTrace,
EmissionTrace,
EnablePulse,
DensityPulse,
EmissionPulse,
EnablePyramids,
DensityPyramids,
EmissionPyramids,
EnablePoints,
DensityPoints,
EmissionPoints,
EnablePoles,
DensityPoles,
EmissionPoles,
VrEyeHeight,
VrEyeBack,
    WindowWidth, WindowHeight, InitialScene,
    TimerVisible, TimerScale, TimerX, TimerY, TimerOpacity,
    PoleAmplitude, PolePeriod, PoleVariation,
    DarkPatternEmission, DarkPatternLines, DarkPatternWidth, DarkPatternStart, DarkPatternReveal,
    ExtensionEnabled, ExtensionStart, ExtensionFade, ExtensionShare, ExtensionModulesStart, ExtensionModulesFade, ExtensionModulesShare,
    ExtensionColourStart, ExtensionColourFade, ExtensionSecondStart, ExtensionSecondFade, ExtensionTint,
    QuietStart, QuietFade, QuietMinimum, ReturnStart, ReturnFade, TunnelStart, TunnelFade,
    ExtensionDensity, MiniPoleMinimum,
    ConeSegmentInterval, ConeSegmentReveal, ConeRotation, ConeSeedDelay,
    PreviousExitStart, PreviousExitDuration, TunnelPieceReveal, TunnelGlowFade,
    RedFogMultiplier, RedAtmosphereMultiplier, LedDensityScale, GenericPieceReveal,
    ConeEmerge, ConeSeparate, ConeDrillRotation, ConeModuleGap,
    TunnelDepartureStart, TunnelDepartureDuration, TunnelDepartureDistance,
    ConeEmission, CrystalInterval, CrystalReveal, CrystalDelay, CrystalVariants, CrystalEmission, CrystalBody, CrystalEdge,
    Count
};
enum class ConfigColour { Body, Fog, Floor, Dark, Hex, Trace, Pulse, Pyramids, Points, Poles, WarmFirst, WarmSecond, CoolFirst, CoolSecond, ConeCyan, ConeMagenta, CrystalCyan, CrystalViolet, Count };
struct SettingDefinition { Setting id; const char* path; double initial,minimum,maximum; int kind; const char* description; };
std::span<const SettingDefinition> visual_setting_definitions();
class VisualConfig {
public:
    VisualConfig();
    double operator[](Setting id) const { return values_[static_cast<std::size_t>(id)]; }
    Color colour(ConfigColour id) const { return colours_[static_cast<std::size_t>(id)]; }
    bool family_enabled(ArtRole role) const;
    double family_density(ArtRole role) const;
    double family_emission(ArtRole role) const;
    Color family_colour(ArtRole role) const;
    Parameters parameters;
    std::string soundtrack{"audio/ChezNous_MASTER.wav"};
private:
    std::array<double,static_cast<std::size_t>(Setting::Count)> values_{};
    std::array<Color,static_cast<std::size_t>(ConfigColour::Count)> colours_{};
    friend VisualConfig parse_visual_config(std::string_view text);
};
VisualConfig parse_visual_config(std::string_view text);
const VisualConfig& default_visual_config();
VisualConfig load_visual_config(const std::filesystem::path& path);
std::string visual_config_json(const VisualConfig& config);
class VisualConfigFile {
public:
    explicit VisualConfigFile(std::filesystem::path path);
    std::optional<VisualConfig> changed(bool force=false);
    const std::filesystem::path& path() const { return path_; }
private:
    std::filesystem::path path_;
    std::filesystem::file_time_type write_time_{};
    std::uintmax_t size_{};
    bool exists_{};
};
}

