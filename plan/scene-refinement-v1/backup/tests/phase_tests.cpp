#include "render/art_direction.hpp"
#include "test_support.hpp"
#include <algorithm>

using namespace fantasynt;
int main() {
    const auto& config=default_visual_config();
    CHECK(config[Setting::OpeningDuration]==360);
    CHECK(directed_composition(150,true,config).weights[1]>0);
    CHECK(directed_composition(284,true,config).weights[2]>0.9f);
    CHECK(extension_visual_energy(216,config)<extension_visual_energy(180,config));
    CHECK_NEAR(extension_visual_energy(240,config),1,0.0001);
    const auto warm=directed_visual_parameters(Parameters{},150,config);
    const auto cool=directed_visual_parameters(Parameters{},185,config);
    CHECK(warm.palette[0].r>warm.palette[0].b);
    CHECK(cool.palette[0].b>cool.palette[0].r);
    auto extension=make_landscape_instances(20261011,4096,true);
    clear_extension_route(extension,config);
    for(const auto& item:extension) {
        const auto& m=item.transform;
        const float rx=2*(std::abs(m[0])+std::abs(m[4])+std::abs(m[8]));
        const float ry=2*(std::abs(m[1])+std::abs(m[5])+std::abs(m[9]));
        if(m[13]+ry>=config[Setting::CorridorFloor]*100&&m[13]-ry<=config[Setting::CorridorCeiling]*100)
            CHECK(std::abs(m[12])-rx>=config[Setting::CorridorWidth]*50);
    }
    OpeningGroup pole;
    pole.role=ArtRole::Pole;
    Instance first;
    first.transform={1,0,0,0, 0,1,0,0, 0,0,1,0, 500,1000,0,1};
    first.padding=0.13f;
    Instance second=first;
    second.padding=0.71f;
    const auto a0=directed_instance(first,pole,0),a10=directed_instance(first,pole,10);
    const auto b0=directed_instance(second,pole,0),b10=directed_instance(second,pole,10);
    CHECK(std::abs(a10.transform[13]-a0.transform[13])>1);
    CHECK(std::abs((a10.transform[13]-a0.transform[13])-(b10.transform[13]-b0.transform[13]))>1);
    for(int time=0;time<240;++time) {
        const auto now=directed_instance(first,pole,time);
        const auto next=directed_instance(first,pole,time+0.01);
        CHECK(std::abs(next.transform[13]-now.transform[13])<1);
        CHECK(std::abs(now.transform[13]-first.transform[13])<=100);
        CHECK(now.transform[12]==first.transform[12] && now.transform[14]==first.transform[14]);
    }
    return test_finish();
}
