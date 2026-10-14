#include "render/art_direction.hpp"
#include "test_support.hpp"
#if __has_include("render/mini_poles.hpp")
#include "render/mini_poles.hpp"
#include <algorithm>

using namespace fantasynt;
int main(int argc,char** argv) {
    CHECK(argc==2);
    auto scene=load_opening_scene(argv[1]);
    const auto original=scene.meshes;
    CHECK(prepare_mini_poles(scene)>0);
    extend_opening_intro(scene);
    clear_directed_route(scene);
    std::size_t checked{};
    for(const auto& group:scene.groups) if(group.floor_anchored) {
        const auto& mesh=scene.meshes.at(group.mesh);
        CHECK(mesh.indices.size()<original.at(group.mesh).indices.size());
        CHECK(group.shaft_height>100 && group.shaft_height<140);
        for(const auto& v:mesh.vertices) {
            if(v.y<0.01) CHECK(std::abs(v.x)<6 && std::abs(v.z)<6);
        }
        for(const auto& source:group.instances) {
            const auto initial=directed_instance(source,group,0);
            const auto later=directed_instance(source,group,10);
            const auto third=directed_instance(source,group,20);
            CHECK(std::max({initial.phase,later.phase,third.phase})-std::min({initial.phase,later.phase,third.phase})>1);
            for(double time:{0.0,10.0,40.0,119.0,180.0,240.0}) {
                const auto animated=directed_instance(source,group,time);
                CHECK_NEAR(animated.transform[13],0,0.0001);
                CHECK(!intersects_directed_route(animated,group,mesh));
                float bottom=100000,top=-100000;
                for(const auto& v:mesh.vertices) {
                    const float local=mini_pole_height(v.y,animated.phase,animated.color_mix);
                    const auto& m=animated.transform;
                    const float world=m[1]*v.x+m[5]*local+m[9]*v.z+m[13];
                    bottom=std::min(bottom,world); top=std::max(top,world);
                }
                CHECK_NEAR(bottom,0,0.001);
                CHECK(top>30);
                // A cabeça desloca-se como bloco: o seu tamanho não acompanha o alongamento.
                CHECK_NEAR(mini_pole_height(140,animated.phase,animated.color_mix)-
                    mini_pole_height(134,animated.phase,animated.color_mix),6,0.001);
            }
            ++checked;
        }
    }
    CHECK(checked>600);
    const auto extreme=parse_visual_config(R"({"schema_version":1,"poles":{"motion_amplitude_m":5,"minimum_height_m":0.05}})");
    for(const auto& group:scene.groups) if(group.floor_anchored) {
        for(float scale:{0.1f,1.0f,10.0f}) {
            auto source=group.instances.front(); source.transform[5]=scale;
            float lowest=0;
            for(int time=0;time<100;++time) {
                const auto animated=directed_instance(source,group,time,extreme);
                for(const auto& v:scene.meshes.at(group.mesh).vertices) {
                    const auto y=mini_pole_height(v.y,animated.phase,animated.color_mix)*scale;
                    lowest=std::min(lowest,y);
                }
            }
            CHECK(lowest>=-0.001f);
        }
        break;
    }
    const auto& config=default_visual_config();
    CHECK(directed_composition(120,true,config).geometry_density_multiplier==1);
    CHECK(directed_composition(150,true,config).geometry_density_multiplier<=0.4);
    CHECK(directed_composition(180,true,config).geometry_density_multiplier<=0.4);
    CHECK(directed_composition(240,true,config).geometry_density_multiplier==1);
    return test_finish();
}
#else
int main() {
    test_check(false,"mini-pilares ainda têm base e flutuam em vez de alongar",__LINE__);
    return test_finish();
}
#endif
