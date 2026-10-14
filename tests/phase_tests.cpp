#include "render/art_direction.hpp"
#include "render/spectral_geometry.hpp"
#include "test_support.hpp"
#include <algorithm>

using namespace fantasynt;
int main() {
    const auto& config=default_visual_config();
    const auto rocks=make_landscape_instances(20261011,4096,false);
    for(const auto& item:rocks) if(item.variation==8) {
        const auto& m=item.transform;
        const auto sx=std::hypot(m[0],m[1],m[2]);
        const auto sy=std::hypot(m[4],m[5],m[6]);
        const auto sz=std::hypot(m[8],m[9],m[10]);
        CHECK(sy<=100);
        CHECK_NEAR(sx,sy,0.001);
        CHECK_NEAR(sz,sy,0.001);
        CHECK_NEAR(m[13],0,0.001);
        float height=0;
        const int variant=static_cast<int>(item.color_mix*crystal_variant_count)%crystal_variant_count;
        for(int piece=0;piece<crystal_piece_count;++piece)
            for(const auto& vertex:make_crystal_piece_geometry(variant,piece).vertices)
                height=std::max(height,vertex.y*m[5]);
        CHECK(height>=config[Setting::CrystalHeightMin]*100-0.001);
        CHECK(height<=config[Setting::CrystalHeightMax]*100+0.001);
    }
    const auto cones=make_landscape_instances(20261011,4096,true);
    const auto cone_count=std::count_if(cones.begin(),cones.end(),[](const auto& item) { return item.variation==9; });
    CHECK(cone_count==224);
    auto separated=rocks;
    clear_extension_route(separated,config);
    separate_ground_rocks(separated,cones,config);
    for(const auto& rock:separated) if(rock.variation==8)
        for(const auto& cone:cones) if(cone.variation==9)
            CHECK(std::hypot(rock.transform[12]-cone.transform[12],rock.transform[14]-cone.transform[14])>=
                1.8*std::hypot(rock.transform[0],rock.transform[2])+1.04*std::hypot(cone.transform[0],cone.transform[2])+config[Setting::ConeGap]*100-0.001);
    for(std::size_t i=0;i<cones.size();++i) if(cones[i].variation==9) {
        const auto& a=cones[i].transform;
        const auto ra=std::hypot(a[0],a[2])*1.04;
        for(std::size_t j=i+1;j<cones.size();++j) if(cones[j].variation==9) {
            const auto& b=cones[j].transform;
            const auto rb=std::hypot(b[0],b[2])*1.04;
            CHECK(std::hypot(a[12]-b[12],a[14]-b[14])>=ra+rb+100);
        }
    }
    OpeningGroup triangle;
    triangle.name="Forest:Scatter_Pyramid_Small:SM_Pyramid_Black";
    triangle.role=ArtRole::Dark; triangle.stage=OpeningStage::Forest;
    CHECK(!directed_group_visible(triangle,180,config));
    OpeningGroup platform=triangle;
    platform.name="Forest:Scatter_Hex:SM_Hex_Solid_1";
    platform.role=ArtRole::Hex;
    CHECK(!directed_group_visible(platform,180,config));
    CHECK(directed_composition(289.9,true,config).tunnel_light_weight==0);
    CHECK_NEAR(config[Setting::OpeningDuration],424.1821315192744,0.000001);
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
        const float rx=item.variation==9?1.04f*std::max(std::hypot(m[0],m[2]),std::hypot(m[8],m[10])):
            2*(std::abs(m[0])+std::abs(m[4])+std::abs(m[8]));
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
