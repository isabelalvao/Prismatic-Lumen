#include "mini_poles.hpp"
#include "art_direction.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>

namespace fantasynt {
float mini_pole_height(float y,float displacement,float shaft_height) {
    return y+displacement*std::clamp(y/shaft_height,0.0f,1.0f);
}
std::size_t prepare_mini_poles(OpeningScene& scene) {
    std::unordered_set<std::string> prepared;
    std::size_t groups{};
    for(auto& group:scene.groups) {
        // SM_Pole_Map já não tem a sapata; só os mini-pilares com sapata são alterados.
        if(group.material!=11||!group.mesh.starts_with("SM_Pole-")) continue;
        auto& mesh=scene.meshes.at(group.mesh);
        if(prepared.insert(group.mesh).second) {
            float width{};
            for(const auto& v:mesh.vertices) if(v.y>0.01f) {
                width=std::max(width,std::max(std::abs(v.x),std::abs(v.z)));
            }
            if(width<=0) throw std::runtime_error("Mini pole: shaft geometry missing");
            SurfaceGeometry clean;
            std::vector<std::uint16_t> remap(mesh.vertices.size(),65535);
            for(std::size_t i=0;i<mesh.indices.size();i+=3) {
                bool base=false;
                for(int j=0;j<3;++j) {
                    const auto& v=mesh.vertices[mesh.indices[i+j]];
                    base|=v.y<0.01f&&std::max(std::abs(v.x),std::abs(v.z))>width*1.05f;
                }
                if(base) continue;
                for(int j=0;j<3;++j) {
                    const auto old=mesh.indices[i+j];
                    if(remap[old]==65535) {
                        remap[old]=static_cast<std::uint16_t>(clean.vertices.size());
                        clean.vertices.push_back(mesh.vertices[old]);
                    }
                    clean.indices.push_back(remap[old]);
                }
            }
            if(clean.vertices.empty()||clean.indices.empty()) throw std::runtime_error("Mini pole: base removal left empty geometry");
            mesh=std::move(clean);
        }
        prepare_art_bounds(group,mesh);
        group.floor_anchored=true;
        group.shaft_height=std::numeric_limits<float>::max();
        for(const auto& v:mesh.vertices) if(v.y>0.01f) group.shaft_height=std::min(group.shaft_height,v.y);
        if(!std::isfinite(group.shaft_height)||group.shaft_height<=0) throw std::runtime_error("Mini pole: invalid shaft height");
        ++groups;
    }
    return groups;
}
}
