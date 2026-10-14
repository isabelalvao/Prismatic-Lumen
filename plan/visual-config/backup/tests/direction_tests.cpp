#include "audio/beat_envelope.hpp"
#include "render/art_direction.hpp"
#include "test_support.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

using namespace fantasynt;
int main(int argc,char** argv) {
    CHECK_NEAR(directed_camera_position(0),-6400.00048828125,0.001);
    CHECK_NEAR(directed_camera_position(10),directed_camera_position(0),0.001);
    CHECK(directed_camera_position(10.1)>directed_camera_position(10));
    CHECK_NEAR(directed_camera_position(21)-directed_camera_position(20),150.0,0.001);
    CHECK_NEAR(directed_camera_position(10.001)-directed_camera_position(10),0,0.001);
    CHECK(directed_camera_step(20,21,0)==0);
    CHECK_NEAR(directed_camera_step(20,21,0.3f),150.0,0.001);
    CHECK(opening_art_presence(ArtRole::Pulse,0,0)==0);
    CHECK(opening_art_presence(ArtRole::Trace,0,0)==0);
    CHECK(opening_art_presence(ArtRole::Point,0,0)>0);
    CHECK(opening_art_presence(ArtRole::Pyramid,79.9,80)==0);
    CHECK(opening_art_presence(ArtRole::Pyramid,84,80)>0);
    CHECK(opening_art_presence(ArtRole::Pyramid,88,80)==1);
    CHECK_NEAR(falling_point_height(2000,10)-falling_point_height(2000,0),-100,0.001);
    CHECK_NEAR(falling_point_height(2000,600),falling_point_height(2000,0),0.001);

    // Duas batidas graves sem um relógio periódico: silêncio e agudos não acendem.
    constexpr int rate=44100;
    std::vector<std::int16_t> pcm(rate*3*2);
    for(int frame=0;frame<rate*3;++frame) {
        const double t=static_cast<double>(frame)/rate;
        double value{};
        for(double onset:{1.0,2.0}) {
            const double dt=t-onset;
            if(dt>=0&&dt<0.2) value+=0.7*std::sin(2*3.141592653589793*65*dt)*std::exp(-dt*24);
        }
        if(t>0.3&&t<0.4) value+=0.1*std::sin(2*3.141592653589793*6000*t);
        pcm[frame*2]=pcm[frame*2+1]=static_cast<std::int16_t>(std::clamp(value,-1.0,1.0)*32767);
    }
    const auto beat=BeatEnvelope::analyze(pcm,rate,2);
    CHECK(beat.sample(0.35)==0 && beat.sample(0.8)==0);
    CHECK(beat.sample(1.05)>0.15f && beat.sample(2.05)>0.15f);
    CHECK(beat.sample(1.6)==0 && beat.sample(2.6)==0);
    CHECK(beat.sample(-1)==0 && beat.sample(std::numeric_limits<double>::quiet_NaN())==0);
    CHECK(beat.beat_count()==2);
    CHECK(BeatEnvelope::analyze(std::vector<std::int16_t>(rate*2),rate,2).beat_count()==0);

    if(argc==2) {
        auto scene=load_opening_scene(argv[1]);
        for(const auto& group:scene.groups) if(group.role==ArtRole::Pyramid) CHECK(group.start_time==80);
        CHECK(extend_opening_intro(scene)==2239*4);
        const auto moved=clear_directed_route(scene);
        CHECK(moved>0);
        for(const auto& group:scene.groups) for(const auto& original:group.instances) {
            for(double time:{80.0,90.0,120.0,600.0}) {
                const auto animated=directed_instance(original,group,time);
                CHECK(!intersects_directed_route(animated,group,scene.meshes.at(group.mesh)));
                if(group.role==ArtRole::Pyramid) CHECK_NEAR(animated.transform[13],original.transform[13],0.001);
            }
        }
    }
    return test_finish();
}
