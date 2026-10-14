#include "render/surface_choreography.hpp"
#include "render/spectral_geometry.hpp"
#include "test_support.hpp"

using namespace fantasynt;

int main() {
    // Regressão: os anéis têm de começar dispersos antes do alinhamento central.
    const auto tunnel = make_directed_tunnel_instances(20261011);
    int scattered_rings = 0;
    for (const auto& item : tunnel) if (item.variation == 5) {
        CHECK(std::hypot(item.transform[12], item.transform[13] - 260) > 500);
        ++scattered_rings;
    }
    CHECK(scattered_rings == 64);
    const auto& config=default_visual_config();
    std::vector<std::array<double,3>> movements;
    for(const auto& source:tunnel) if(source.variation!=5&&movements.size()<2) {
        const auto a=directed_tunnel_instance(source,240,40000,40000,config);
        const auto b=directed_tunnel_instance(source,241,40000,40000,config);
        std::array<double,3> delta{b.transform[12]-a.transform[12],b.transform[13]-a.transform[13],b.transform[14]-a.transform[14]};
        CHECK(std::hypot(delta[0],delta[1],delta[2])>0.01);
        movements.push_back(delta);
    }
    CHECK(std::hypot(movements[0][0]-movements[1][0],movements[0][1]-movements[1][1],movements[0][2]-movements[1][2])>0.01);
    // A distância é medida até à última parte da peça: conserva 60 m e dissolve em 30 m.
    CHECK_NEAR(rear_piece_visibility(-6000,0,config),1,1e-9);
    CHECK_NEAR(rear_piece_visibility(-7500,0,config),0.5,1e-9);
    CHECK_NEAR(rear_piece_visibility(-9000,0,config),0,1e-9);
    CHECK_NEAR(rear_piece_visibility(-6500,500,config),1,1e-9);
    CHECK_NEAR(rear_piece_visibility(-8000,500,config),0.5,1e-9);
    CHECK_NEAR(retained_wrapped_depth(-6000,500,config),-6000,1e-9);
    CHECK_NEAR(retained_wrapped_depth(-9400,500,config),-9400,1e-9);
    CHECK(retained_wrapped_depth(-9501,500,config)>40000);
    const std::array<double,4> attacks{136.1,136.6,137.1,138.0};
    CHECK_NEAR(beat_aligned_time(136.2,attacks),136.6,1e-9);
    CHECK_NEAR(beat_aligned_time(136.6,attacks),136.6,1e-9);
    CHECK_NEAR(beat_aligned_time(138.1,attacks),138.1,1e-9);
    CHECK_NEAR(beat_aligned_time(130,attacks),130,1e-9);
    CHECK_NEAR(beat_aligned_time(136.2,{}),136.2,1e-9);
    const auto torus=make_torus_geometry();
    // Mesmo acelerando motion_speed de 0,3 para 1, nenhum anel em formação alcança a cabeça.
    for(const auto& source:tunnel) if(source.variation==5) {
        for(double time=290;time<=306;time+=0.25) {
            const double fast_camera=40000+(time-290)*500;
            const auto moving=directed_tunnel_instance(source,time,fast_camera,40000,config);
            if(moving.transform[14]<1000) {
                CHECK_NEAR(moving.transform[12],0,0.001);
                CHECK_NEAR(moving.transform[13],260,0.001);
                CHECK(moving.transform[0]*0.796>=499.99);
            }
        }
    }
    double previous_z=-1e9;
    for(const auto& source:tunnel) if(source.variation==5) {
        const auto before=directed_tunnel_instance(source,289,40000,40000,config);
        CHECK_NEAR(before.transform[12],source.transform[12],0.001);
        CHECK_NEAR(before.transform[0],source.transform[0],0.001);
        const auto during=directed_tunnel_instance(source,300,41500,40000,config);
        CHECK(std::abs(during.transform[12])<std::abs(source.transform[12]));
        CHECK(during.transform[0]>source.transform[0]);
        const auto complete=directed_tunnel_instance(source,306,42400,40000,config);
        CHECK_NEAR(complete.transform[12],0,0.001);
        CHECK_NEAR(complete.transform[13],260,0.001);
        CHECK(complete.transform[14]>1500);
        if(previous_z>-1e8) CHECK_NEAR(complete.transform[14]-previous_z,800,0.01);
        previous_z=complete.transform[14];
        // Vérifie le trou réel après l'amincissement appliqué par le shader.
        for(const auto& v:torus.vertices) {
            const double radius=(0.82+(std::hypot(v.x,v.y)-0.82)*0.2)*complete.transform[0];
            CHECK(radius>=499.99);
        }
    }
    const auto wide=parse_visual_config(R"({"schema_version":1,"layout":{"corridor_width_m":15,"corridor_ceiling_m":12},"extension":{"ring_inner_radius_m":1}})");
    const auto fast_close=parse_visual_config(R"({"schema_version":1,"camera":{"speed_m_s":10},"extension":{"ring_spacing_m":1}})");
    CHECK_NEAR(required_ring_lead(5,config),50,1e-9);
    CHECK_NEAR(required_ring_lead(10/0.3,fast_close),480.333333333333,1e-8);
    for(const auto& source:tunnel) if(source.variation==5) {
        for(double time=290;time<=306;time+=0.25) {
            const double camera=40000+(time-290)*1000/0.3;
            const auto moving=directed_tunnel_instance(source,time,camera,40000,fast_close);
            if(moving.transform[14]<1000) {
                CHECK_NEAR(moving.transform[12],0,0.001);
                CHECK_NEAR(moving.transform[13],260,0.001);
                CHECK(moving.transform[0]*0.796>=499.99);
            }
        }
    }
    // Uma alteração extrema de velocidade a meio não muda a origem do túnel e antecipa a folga.
    for(const auto& source:tunnel) if(source.variation==5) {
        for(double time=293;time<=306;time+=0.05) {
            const double camera=41500+(time-293)*1000/0.3;
            const auto moving=directed_tunnel_instance(source,time,camera,40000,fast_close,1,50);
            if(moving.transform[14]<1000) {
                CHECK_NEAR(moving.transform[12],0,0.001);
                CHECK_NEAR(moving.transform[13],260,0.001);
                CHECK(moving.transform[0]*0.796>=499.99);
            }
        }
    }
    for(const auto& source:tunnel) if(source.variation==5) {
        const auto complete=directed_tunnel_instance(source,306,42400,40000,wide);
        CHECK(complete.transform[0]*0.796>std::hypot(770.0,940.0));
        break;
    }
    SurfaceChoreographySettings settings;
    Instance authored;
    authored.transform={500,0,0,0, 0,900,0,0, 0,0,500,0, 0,0,3000,1};
    for(double opacity:{0.0,0.1,0.5,1.0}) {
        const auto faded=surface_fade_instance(authored,opacity);
        for(int col=0;col<3;++col) for(int row=0;row<3;++row)
            CHECK(faded.transform[col*4+row]==authored.transform[col*4+row]);
        for(int row=12;row<16;++row) CHECK(faded.transform[row]==authored.transform[row]);
        CHECK_NEAR(faded.transform[7],opacity,1e-6);
    }
    CHECK_NEAR(spectral_piece_visibility(8,0,151.9,136,2,0.8,2),0,1e-9);
    CHECK_NEAR(spectral_piece_visibility(8,0,152.4,136,2,0.8,2),0.5,1e-9);
    CHECK_NEAR(spectral_piece_visibility(8,1,154.8,136,2,0.8,2),1,1e-9);
    // A ponta emerge primeiro; nenhum módulo se separa ou contrarroda sob o piso.
    const auto buried=drill_cone_motion(0,136,settings);
    const auto rising=drill_cone_motion(0,143,settings);
    const auto surface=drill_cone_motion(0,150,settings);
    const auto separated=drill_cone_motion(0,153,settings);
    CHECK_NEAR(buried.rise,0,1e-9);
    CHECK_NEAR(rising.rise,0.5,1e-9);
    CHECK(rising.split==0 && surface.split==0);
    CHECK_NEAR(surface.rise,1,1e-9);
    CHECK_NEAR(separated.split,1,1e-9);
    CHECK_NEAR(drill_cone_motion(1,152,settings).rise,1,1e-9);
    CHECK_NEAR(cone_module_local_offset(6,buried),-1.922857142857,1e-8);
    CHECK_NEAR(cone_module_local_offset(0,surface),0,1e-9);
    CHECK_NEAR(cone_module_local_offset(6,separated),0,1e-9);
    CHECK_NEAR(tunnel_departure_progress(0,269.99,270,30),0,1e-9);
    CHECK_NEAR(tunnel_departure_progress(0,270,270,30),0,1e-9);
    CHECK(tunnel_departure_progress(0,285,270,30)>0);
    CHECK_NEAR(tunnel_departure_progress(1,300,270,30),1,1e-9);
    // Cada anel e as suas duas tampas pertencem ao mesmo segmento.
    for (int segment = 0; segment < 7; ++segment) {
        CHECK(cone_segment_index(segment * 2.0 / 7.0) == segment);
        CHECK(cone_segment_index((segment + 0.73) * 2.0 / 7.0) == segment);
    }
    CHECK(cone_segment_index(-1) == 0);
    CHECK(cone_segment_index(2) == 6);
    CHECK_NEAR(cone_segment_visibility(0, 0, 135, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 0, 136.4, settings), 0.5, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 0, 136.8, settings), 1, 1e-9);
    CHECK_NEAR(cone_segment_visibility(1, 0, 137.9, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_visibility(6, 1, 150.8, settings), 1, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 1, 136.8, settings), 0, 1e-9);

    settings.cone_rotation_start_clock = 40;
    CHECK_NEAR(cone_segment_angle(0, 0, 40, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(0, 0, 40.8, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(0, 0, 41.8, settings), 0.034906585, 1e-9);
    CHECK_NEAR(cone_segment_angle(1, 0, 43.8, settings), -0.034906585, 1e-9);
    CHECK_NEAR(cone_segment_angle(2, 1, 45, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(2, 1, 47.8, settings), 0.034906585, 1e-9);
    // A rotação usa exclusivamente o relógio de animação pausável.
    CHECK_NEAR(cone_segment_angle(6, 0, 52, settings), 0, 1e-9);

    CHECK_NEAR(previous_piece_visibility(0, 0), 1, 1e-9);
    CHECK_NEAR(previous_piece_visibility(0, 0.35), 0, 1e-9);
    CHECK_NEAR(previous_piece_visibility(1, 0.65), 1, 1e-9);
    CHECK_NEAR(previous_piece_visibility(1, 1), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.4, 0.99, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224, 1, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.4, 1, settings), 0.5, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.8, 1, settings), 1, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0.5, 239.5, 1, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(1, 272, 1, settings), 1, 1e-9);
    CHECK_NEAR(generic_piece_visibility(1, 172, 172, settings), 0, 1e-9);
    CHECK_NEAR(generic_piece_visibility(0, 172.4, 172, settings), 0.5, 1e-9);
    CHECK_NEAR(generic_piece_visibility(1, 180, 172, settings), 1, 1e-9);
    settings.segment_reveal_seconds = 0;
    settings.tube_piece_reveal_seconds = 0;
    CHECK_NEAR(cone_segment_visibility(0, 0, 136, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224, 1, settings), 0, 1e-9);
    CHECK_NEAR(generic_piece_visibility(0, 172, 172, settings), 0, 1e-9);
    return test_finish();
}
