#include "render/art_direction.hpp"
#include "test_support.hpp"
#if __has_include("render/mini_poles.hpp")
#include "render/mini_poles.hpp"
#include <algorithm>

using namespace fantasynt;
namespace {
std::array<float,3> world_point(const SurfaceVertex& v,const Instance& instance) {
    const auto& m=instance.transform;
    const float y=mini_pole_height(v.y,instance.phase,instance.color_mix);
    return {m[0]*v.x+m[4]*y+m[8]*v.z+m[12],
        m[1]*v.x+m[5]*y+m[9]*v.z+m[13],
        m[2]*v.x+m[6]*y+m[10]*v.z+m[14]};
}
float distance(const std::array<float,3>& a,const std::array<float,3>& b) {
    return std::sqrt((a[0]-b[0])*(a[0]-b[0])+(a[1]-b[1])*(a[1]-b[1])+(a[2]-b[2])*(a[2]-b[2]));
}
void check_rigid_translation(const SurfaceGeometry& mesh,const Instance& baseline,const Instance& animated) {
    const auto anchor=world_point(mesh.vertices.front(),baseline);
    const auto moved_anchor=world_point(mesh.vertices.front(),animated);
    for(const auto& vertex:mesh.vertices) {
        const auto before=world_point(vertex,baseline),after=world_point(vertex,animated);
        // A mesma translação em todos os vértices conserva a distância de qualquer par.
        CHECK_NEAR(after[0]-before[0],0,0.002f);
        CHECK_NEAR(after[1]-before[1],moved_anchor[1]-anchor[1],0.002f);
        CHECK_NEAR(after[2]-before[2],0,0.002f);
        CHECK_NEAR(distance(after,moved_anchor),distance(before,anchor),0.003f);
    }
    for(int i=0;i<12;++i) CHECK(animated.transform[i]==baseline.transform[i]);
}
}
int main(int argc,char** argv) {
    CHECK(argc==2);
    // A haste e a cabeça mantêm a separação mesmo num deslocamento grande.
    CHECK_NEAR(mini_pole_height(140,30,120)-mini_pole_height(0,30,120),140,0.001f);
    auto scene=load_opening_scene(argv[1]);
    const auto original=scene.meshes;
    CHECK(prepare_mini_poles(scene)>0);
    extend_opening_intro(scene);
    clear_directed_route(scene);
    const auto reloaded_config=parse_visual_config(R"({"schema_version":1,"poles":{"motion_period_seconds":9,"period_variation":0.67}})");
    auto motion_before=sample_art_motion(17.3,default_visual_config());
    motion_before.pole_phase_base=0.231;
    motion_before.pole_phase_seed=-0.193;
    auto motion_after=sample_art_motion(17.3,reloaded_config);
    const double old_variation=default_visual_config()[Setting::PoleVariation];
    const double new_variation=reloaded_config[Setting::PoleVariation];
    // Rebase do relógio ao recarregar: mantém a fase de cada frequência individual.
    motion_after.pole_phase_base=motion_before.pole_phase_base+
        motion_before.poles*(1-old_variation)-motion_after.poles*(1-new_variation);
    motion_after.pole_phase_seed=motion_before.pole_phase_seed+
        2*(motion_before.poles*old_variation-motion_after.poles*new_variation);
    std::size_t checked{};
    for(const auto& group:scene.groups) if(group.floor_anchored) {
        const auto& mesh=scene.meshes.at(group.mesh);
        CHECK(mesh.indices.size()<original.at(group.mesh).indices.size());
        CHECK(group.shaft_height>100 && group.shaft_height<140);
        for(const auto& v:mesh.vertices) {
            if(v.y<0.01) CHECK(std::abs(v.x)<6 && std::abs(v.z)<6);
        }
        for(const auto& source:group.instances) {
            const auto before_reload=directed_instance_with_motion(source,group,motion_before,default_visual_config());
            const auto after_reload=directed_instance_with_motion(source,group,motion_after,reloaded_config);
            CHECK_NEAR(before_reload.transform[13],after_reload.transform[13],0.001f);
            check_rigid_translation(mesh,before_reload,after_reload);
            const auto initial=directed_instance(source,group,0);
            const auto later=directed_instance(source,group,10);
            const auto third=directed_instance(source,group,20);
            CHECK(std::max({initial.transform[13],later.transform[13],third.transform[13]})-
                std::min({initial.transform[13],later.transform[13],third.transform[13]})>1);
            for(double time:{0.0,10.0,40.0,119.0,180.0,240.0}) {
                const auto animated=directed_instance(source,group,time);
                CHECK(animated.phase==0);
                CHECK(animated.color_mix==group.shaft_height);
                CHECK(!intersects_directed_route(animated,group,mesh));
                float top=-100000;
                for(const auto& v:mesh.vertices) top=std::max(top,world_point(v,animated)[1]);
                CHECK(top>=default_visual_config()[Setting::MiniPoleMinimum]*100-0.001);
                check_rigid_translation(mesh,initial,animated);
            }
            ++checked;
        }
    }
    CHECK(checked>600);
    const auto extreme=parse_visual_config(R"({"schema_version":1,"poles":{"motion_amplitude_m":5,"minimum_height_m":0.05}})");
    for(const auto& group:scene.groups) if(group.floor_anchored) {
        for(float scale:{0.1f,1.0f,10.0f}) {
            auto source=group.instances.front(); source.transform[5]=scale;
            const auto baseline=directed_instance(source,group,0,extreme);
            bool reached_minimum=false;
            for(int time=0;time<100;++time) {
                const auto animated=directed_instance(source,group,time,extreme);
                const auto& mesh=scene.meshes.at(group.mesh);
                float top=-100000;
                for(const auto& v:mesh.vertices) top=std::max(top,world_point(v,animated)[1]);
                CHECK(top>=5-0.001f);
                reached_minimum|=std::abs(top-5)<0.001f;
                check_rigid_translation(mesh,baseline,animated);
            }
            if(scale<1) CHECK(reached_minimum);
        }
        break;
    }
    // Os ficheiros de referência conservam a sapata e todos os vértices originais.
    const auto reloaded=load_opening_scene(argv[1]);
    for(const auto& [name,mesh]:original) {
        const auto& on_disk=reloaded.meshes.at(name);
        CHECK(mesh.indices==on_disk.indices);
        CHECK(mesh.vertices.size()==on_disk.vertices.size());
        for(std::size_t i=0;i<mesh.vertices.size();++i) {
            CHECK(mesh.vertices[i].x==on_disk.vertices[i].x);
            CHECK(mesh.vertices[i].y==on_disk.vertices[i].y);
            CHECK(mesh.vertices[i].z==on_disk.vertices[i].z);
        }
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
    test_check(false,"mini-pilares sem preparação para translação vertical rígida",__LINE__);
    return test_finish();
}
#endif
