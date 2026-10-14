#include "visual_config.hpp"
#include "control/protocol.hpp"
#include "render/opening_scene.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <stdexcept>
#include <unordered_set>

namespace fantasynt {
namespace {
using Json=nlohmann::json;
#include "visual_settings.inc"
Color read_colour(const Json& value) {
    const Json command={{"schema_version",1},{"id","visual-colour"},{"command","apply_parameters"},
        {"transition_seconds",0.1},{"parameters",{{"palette",Json::array({value,value})}}}};
    const auto parsed=parse_command(command.dump());
    if(const auto* result=std::get_if<Command>(&parsed)) return result->parameters.palette->at(0);
    throw std::runtime_error("Visual config: colour must be #RRGGBB");
}
void put(Json& root,std::string_view path,const Json& value) {
    const auto dot=path.find('.');
    if(dot==path.npos) { root[std::string(path)]=value; return; }
    put(root[std::string(path.substr(0,dot))],path.substr(dot+1),value);
}
bool known_prefix(const std::string& prefix) {
    for(const auto& item:settings) if(std::string_view(item.path).starts_with(prefix+".")) return true;
    for(const auto* path:colour_paths) if(std::string_view(path).starts_with(prefix+".")) return true;
    return false;
}
void validate_relations(const VisualConfig& c) {
    const auto require=[](bool ok,const char* reason) { if(!ok) throw std::runtime_error(std::string("Visual config: ")+reason); };
    require(c[Setting::FarPlane]>c[Setting::NearPlane],"far_plane must exceed near_plane");
    require(c[Setting::TintEnd]>c[Setting::TintStart],"palette_depth_end must exceed start");
    require(c[Setting::SkyOuter]>c[Setting::SkyInner],"sky outer radius must exceed inner radius");
    require(c[Setting::TraceUpper]>c[Setting::TraceLower],"dash_end must exceed dash_start");
    require(c[Setting::PyramidWidthMax]>=c[Setting::PyramidWidthMin],"line_width_max must exceed min");
    require(c[Setting::BassHigh]>c[Setting::BassLow],"high_frequency_hz must exceed low_frequency_hz");
    require(c[Setting::PulseTail]>c[Setting::PulseAttack],"tail_seconds must exceed attack_seconds");
    require(c[Setting::CorridorCeiling]>c[Setting::CorridorFloor],"corridor ceiling must exceed floor");
    require(c[Setting::PointEdge]*2<c[Setting::PointHeight],"edge_fade_m must be less than half cycle_height_m");
    require(c[Setting::PointFinal]>=c[Setting::PointInitial],"points_final_presence must exceed initial");
    require(c[Setting::ConeSegmentReveal]<=c[Setting::ConeSegmentInterval],"cone reveal must not exceed segment interval");
    require(c[Setting::PreviousExitStart]+c[Setting::PreviousExitDuration]<=c[Setting::TunnelStart],"previous pieces must finish exiting before tunnel starts");
    require(c[Setting::TunnelPieceReveal]<=c[Setting::TunnelFade],"tunnel piece reveal must not exceed construction time");
    require(c[Setting::GenericPieceReveal]>=c[Setting::ConeSegmentReveal],"other piece window must cover reveal duration");
    require(c[Setting::ArtifactSpeedMax]>=c[Setting::ArtifactSpeedMin],"maximum artifact speed must exceed minimum");
    require(c[Setting::CrystalHeightMax]>=c[Setting::CrystalHeightMin],"maximum rock height must exceed minimum");
}
}
std::span<const SettingDefinition> visual_setting_definitions() { return settings; }
const VisualConfig& default_visual_config() { static const VisualConfig config; return config; }
VisualConfig::VisualConfig() {
    for(const auto& d:settings) values_[static_cast<std::size_t>(d.id)]=d.initial;
    for(std::size_t i=0;i<colours_.size();++i) colours_[i]=read_colour(colour_defaults[i]);
}
bool VisualConfig::family_enabled(ArtRole role) const {
    static constexpr Setting ids[]={Setting::EnableDark,Setting::EnableHex,Setting::EnableTrace,Setting::EnablePulse,Setting::EnablePyramids,Setting::EnablePoints,Setting::EnablePoles};
    return (*this)[ids[static_cast<std::size_t>(role)]]!=0;
}
double VisualConfig::family_density(ArtRole role) const {
    static constexpr Setting ids[]={Setting::DensityDark,Setting::DensityHex,Setting::DensityTrace,Setting::DensityPulse,Setting::DensityPyramids,Setting::DensityPoints,Setting::DensityPoles};
    return (*this)[ids[static_cast<std::size_t>(role)]];
}
double VisualConfig::family_emission(ArtRole role) const {
    static constexpr Setting ids[]={Setting::EmissionDark,Setting::EmissionHex,Setting::EmissionTrace,Setting::EmissionPulse,Setting::EmissionPyramids,Setting::EmissionPoints,Setting::EmissionPoles};
    return (*this)[ids[static_cast<std::size_t>(role)]];
}
Color VisualConfig::family_colour(ArtRole role) const { return colours_[3+static_cast<std::size_t>(role)]; }
VisualConfig parse_visual_config(std::string_view text) {
    if(text.empty()||text.size()>65536) throw std::runtime_error("Visual config: file must contain 1..65536 bytes");
    std::vector<std::unordered_set<std::string>> keys;
    const auto callback=[&](int,Json::parse_event_t event,Json& value) {
        if(event==Json::parse_event_t::object_start) keys.emplace_back();
        if(event==Json::parse_event_t::key&&!keys.back().insert(value.get<std::string>()).second)
            throw std::runtime_error("Visual config: duplicate key "+value.get<std::string>());
        if(event==Json::parse_event_t::object_end) keys.pop_back();
        return true;
    };
    const auto doc=Json::parse(text.begin(),text.end(),callback);
    if(!doc.is_object()||!doc.contains("schema_version")||!doc["schema_version"].is_number_integer()||doc["schema_version"]!=1)
        throw std::runtime_error("Visual config: expected schema_version 1");
    VisualConfig result;
    std::function<void(const Json&,const std::string&)> visit;
    visit=[&](const Json& value,const std::string& path) {
        if(path=="startup.soundtrack") {
            if(!value.is_string()||value.get_ref<const std::string&>().size()>4096) throw std::runtime_error("Visual config: soundtrack must be a local path");
            result.soundtrack=value.get<std::string>(); return;
        }
        if(value.is_object()) {
            if(!path.empty()&&!known_prefix(path)) throw std::runtime_error("Visual config: unknown section "+path);
            for(const auto& [key,item]:value.items()) {
                if(path.empty()&&(key=="schema_version"||key=="parameters")) continue;
                visit(item,path.empty()?key:path+"."+key);
            }
            return;
        }
        for(std::size_t i=0;i<std::size(colour_paths);++i) if(path==colour_paths[i]) { result.colours_[i]=read_colour(value); return; }
        const auto* found=std::find_if(std::begin(settings),std::end(settings),[&](const auto& d) { return path==d.path; });
        if(found==std::end(settings)) throw std::runtime_error("Visual config: unknown field "+path);
        double number{};
        if(found->kind==2) {
            if(!value.is_boolean()) throw std::runtime_error("Visual config: expected boolean at "+path);
            number=value.get<bool>()?1:0;
        } else if(found->kind==3) {
            if(!value.is_string()) throw std::runtime_error("Visual config: direction must be alternating, up or down");
            const auto mode=value.get<std::string>();
            if(mode=="alternating") number=0; else if(mode=="up") number=1; else if(mode=="down") number=-1;
            else throw std::runtime_error("Visual config: direction must be alternating, up or down");
        } else if(found->kind==4) {
            const char* scenes[]={"opening","sequence","night","landscape","tunnel","field"};
            if(!value.is_string()) throw std::runtime_error("Visual config: invalid startup.scene");
            const auto scene=value.get<std::string>();
            const auto item=std::find_if(std::begin(scenes),std::end(scenes),[&](auto s) { return scene==s; });
            if(item==std::end(scenes)) throw std::runtime_error("Visual config: invalid startup.scene");
            number=static_cast<double>(item-std::begin(scenes));
        } else {
            if(!value.is_number()) throw std::runtime_error("Visual config: expected number at "+path);
            number=value.get<double>();
        }
        if(!std::isfinite(number)||number<found->minimum||number>found->maximum||(found->kind==1&&number!=std::floor(number)))
            throw std::runtime_error("Visual config: value outside range at "+path);
        result.values_[static_cast<std::size_t>(found->id)]=number;
    };
    visit(doc,"");
    if(doc.contains("parameters")) {
        const Json command={{"schema_version",1},{"id","visual-parameters"},{"command","apply_parameters"},
            {"transition_seconds",0.1},{"parameters",doc["parameters"]}};
        const auto parsed=parse_command(command.dump());
        const auto* change=std::get_if<Command>(&parsed);
        if(!change) throw std::runtime_error("Visual config: invalid global parameters");
        const auto& p=change->parameters;
        if(p.palette) result.parameters.palette=*p.palette;
        if(p.light_intensity) result.parameters.light_intensity=*p.light_intensity;
        if(p.motion_speed) result.parameters.motion_speed=*p.motion_speed;
        if(p.pulse_strength) result.parameters.pulse_strength=*p.pulse_strength;
        if(p.fog_density) result.parameters.fog_density=*p.fog_density;
        if(p.geometry_density) result.parameters.geometry_density=*p.geometry_density;
        if(p.bloom) result.parameters.bloom=*p.bloom;
    }
    validate_relations(result);
    return result;
}
VisualConfig load_visual_config(const std::filesystem::path& path) {
    std::ifstream file(path,std::ios::binary|std::ios::ate);
    if(!file||file.tellg()<=0||file.tellg()>65536) throw std::runtime_error("Visual config: missing, empty or oversized file");
    std::string text(static_cast<std::size_t>(file.tellg()),'\0');
    file.seekg(0); file.read(text.data(),text.size());
    if(!file) throw std::runtime_error("Visual config: cannot read file");
    return parse_visual_config(text);
}
std::string visual_config_json(const VisualConfig& c) {
    Json root={{"schema_version",1},{"parameters",parameters_to_json(c.parameters)}};
    for(const auto& d:settings) {
        const double value=c[d.id];
        if(d.kind==2) put(root,d.path,value!=0);
        else if(d.kind==3) put(root,d.path,value>0?"up":value<0?"down":"alternating");
        else if(d.kind==4) { const char* scenes[]={"opening","sequence","night","landscape","tunnel","field"}; put(root,d.path,scenes[static_cast<int>(value)]); }
        else put(root,d.path,value);
    }
    for(std::size_t i=0;i<std::size(colour_paths);++i) {
        Parameters colours; colours.palette[0]=c.colour(static_cast<ConfigColour>(i));
        put(root,colour_paths[i],parameters_to_json(colours)["palette"][0]);
    }
    put(root,"startup.soundtrack",c.soundtrack);
    return root.dump(2)+"\n";
}
VisualConfigFile::VisualConfigFile(std::filesystem::path path):path_(std::move(path)) {
    std::error_code error;
    write_time_=std::filesystem::last_write_time(path_,error);
    exists_=!error;
    if(exists_) size_=std::filesystem::file_size(path_,error);
}
std::optional<VisualConfig> VisualConfigFile::changed(bool force) {
    std::error_code error;
    const auto stamp=std::filesystem::last_write_time(path_,error);
    const bool exists=!error;
    const auto size=exists?std::filesystem::file_size(path_,error):0;
    if(!force&&exists==exists_&&(!exists||(stamp==write_time_&&size==size_))) return std::nullopt;
    write_time_=stamp; size_=size; exists_=exists;
    return load_visual_config(path_);
}
}
