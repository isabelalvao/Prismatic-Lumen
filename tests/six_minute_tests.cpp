#include "render/art_direction.hpp"
#include "test_support.hpp"
using namespace fantasynt;
int main() {
    const auto& c=default_visual_config();
    CHECK(c[Setting::OpeningDuration]>424);
    CHECK(directed_composition(412,true,c).exposure<directed_composition(408,true,c).exposure);
    CHECK_NEAR(directed_composition(424.1821315192744,true,c).exposure,0,1e-9);
    const auto before=directed_composition(223.9,true,c);
    const auto start=directed_composition(224,true,c);
    const auto halfway=directed_composition(240,true,c);
    const auto built=directed_composition(272,true,c);
    CHECK(before.weights[2]==0);
    CHECK(start.opening_weight==0 && start.weights[0]==0 && start.weights[1]==0);
    CHECK(halfway.weights[2]>0 && halfway.weights[2]<1);
    CHECK(built.weights[2]==1);
    CHECK(directed_composition(271.99,true,c).tunnel_light_weight==0);
    CHECK(built.tunnel_light_weight==0);
    CHECK(directed_composition(290,true,c).tunnel_light_weight==0);
    CHECK(directed_composition(294,true,c).tunnel_light_weight==0);
    CHECK(directed_composition(300,true,c).tunnel_light_weight>0);
    CHECK(directed_composition(306,true,c).tunnel_light_weight==1);
    CHECK(directed_composition(180,true,c).fog_multiplier<1);
    CHECK(directed_composition(180,true,c).atmosphere.r<sample_sequence(180).atmosphere.r);
    CHECK_NEAR(directed_composition(207.999,true,c).atmosphere.r,directed_composition(208,true,c).atmosphere.r,0.001);
    CHECK_NEAR(directed_composition(208,true,c).atmosphere.r,directed_composition(208.001,true,c).atmosphere.r,0.001);
    CHECK_NEAR(directed_camera_position(360,c),-6400.00048828125+54000,0.001);
    return test_finish();
}
