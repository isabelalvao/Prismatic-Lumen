#include "opening_scene.hpp"
#include "art_direction.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <fstream>
#include <stdexcept>

namespace fantasynt {
namespace {
std::vector<char> read_file(const std::filesystem::path& path,std::size_t limit) {
    std::ifstream f(path,std::ios::binary|std::ios::ate);
    if(!f) throw std::runtime_error("Opening resource missing: "+path.string());
    const auto length=f.tellg();
    if(length<=0 || static_cast<std::uint64_t>(length)>limit) throw std::runtime_error("Invalid opening resource size");
    std::vector<char> data(static_cast<std::size_t>(length));
    f.seekg(0); f.read(data.data(),length);
    if(!f) throw std::runtime_error("Cannot read opening resource");
    return data;
}
bool valid_id(const std::string& id) {
    return !id.empty() && id.size()<128 && std::all_of(id.begin(),id.end(),[](unsigned char c) {
        return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-';
    });
}
}
bool opening_visible(OpeningStage stage,double time) {
    if(!std::isfinite(time)||time<0) return false;
    if(stage==OpeningStage::Intro) return time<112;
    if(stage==OpeningStage::Dome) return time<80;
    return time>=80;
}
SurfaceGeometry load_reference_mesh(const std::filesystem::path& path) {
    const auto data=read_file(path,32*1024*1024);
    if(data.size()<16 || std::memcmp(data.data(),"FNTMESH1",8)) throw std::runtime_error("Invalid reference mesh header");
    std::uint32_t vertices{},indices{};
    std::memcpy(&vertices,data.data()+8,4); std::memcpy(&indices,data.data()+12,4);
    if(!vertices||vertices>65535||!indices||indices>1000000||indices%3||
        data.size()!=16+static_cast<std::size_t>(vertices)*sizeof(SurfaceVertex)+static_cast<std::size_t>(indices)*2)
        throw std::runtime_error("Invalid reference mesh counts");
    static_assert(sizeof(SurfaceVertex)==32);
    SurfaceGeometry mesh;
    mesh.vertices.resize(vertices); mesh.indices.resize(indices);
    std::memcpy(mesh.vertices.data(),data.data()+16,vertices*sizeof(SurfaceVertex));
    std::memcpy(mesh.indices.data(),data.data()+16+vertices*sizeof(SurfaceVertex),indices*2);
    for(const auto& v:mesh.vertices) {
        const float values[]={v.x,v.y,v.z,v.nx,v.ny,v.nz,v.u,v.v};
        for(float x:values) if(!std::isfinite(x)) throw std::runtime_error("Non-finite reference mesh");
        const auto n=std::hypot(v.nx,v.ny,v.nz);
        if(n<0.95f||n>1.05f) throw std::runtime_error("Invalid reference mesh normal");
    }
    for(auto i:mesh.indices) if(i>=vertices) throw std::runtime_error("Invalid reference mesh index");
    return mesh;
}
OpeningScene load_opening_scene(const std::filesystem::path& directory) {
    const auto data=read_file(directory/"scene.json",32*1024*1024);
    const auto json=nlohmann::json::parse(data.begin(),data.end());
    if(json.at("schema_version")!=1 || json.at("groups").size()>512) throw std::runtime_error("Invalid opening scene schema");
    OpeningScene result;
    std::size_t count{};
    for(const auto& raw:json.at("groups")) {
        OpeningGroup group;
        group.name=raw.at("name").get<std::string>(); group.mesh=raw.at("mesh").get<std::string>();
        if(!valid_id(group.mesh)) throw std::runtime_error("Invalid opening mesh id");
        const auto stage=raw.at("stage").get<std::string>();
        if(stage=="intro") group.stage=OpeningStage::Intro;
        else if(stage=="dome") group.stage=OpeningStage::Dome;
        else if(stage=="forest") group.stage=OpeningStage::Forest;
        else throw std::runtime_error("Invalid opening stage");
        group.material=raw.value("material",0); group.cull_distance=raw.value("cull_distance",16000.0f);
        group.start_time=raw.value("start_time",0.0);
        if(group.material<0||group.material>31||!std::isfinite(group.cull_distance)||group.cull_distance<0||!std::isfinite(group.start_time))
            throw std::runtime_error("Invalid opening group material/culling");
        if(group.cull_distance==0) group.cull_distance=32000;
        if(!result.meshes.contains(group.mesh)) result.meshes.emplace(group.mesh,
            load_reference_mesh(directory/"meshes"/(group.mesh+".fmesh")));
        for(const auto& transform:raw.at("instances")) {
            if(transform.size()!=16 || ++count>150000) throw std::runtime_error("Invalid opening instance count/matrix");
            Instance instance;
            for(int i=0;i<16;++i) {
                const float x=transform.at(i).get<float>();
                if(!std::isfinite(x)||std::abs(x)>1000000) throw std::runtime_error("Invalid opening transform");
                instance.transform[i]=x;
            }
            if(instance.transform[3]!=0||instance.transform[7]!=0||instance.transform[11]!=0||instance.transform[15]!=1)
                throw std::runtime_error("Opening transform must be affine");
            instance.variation=static_cast<float>(group.material);
            instance.padding=static_cast<float>(group.instances.size()%1024)/1024;
            group.instances.push_back(instance);
        }
        if(group.instances.empty()) throw std::runtime_error("Empty opening group");
        prepare_art_bounds(group,result.meshes.at(group.mesh));
        result.groups.push_back(std::move(group));
    }
    if(result.groups.empty()) throw std::runtime_error("Empty opening scene");
    return result;
}
}
