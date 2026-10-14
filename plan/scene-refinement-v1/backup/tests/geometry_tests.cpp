#include "render/geometry.hpp"
#include "test_support.hpp"
#include <cmath>

using namespace fantasynt;
int main() {
    auto a=make_instances(20261011,4096);
    auto b=make_instances(20261011,4096);
    auto c=make_instances(7,4096);
    CHECK(a.size()==4096);
    CHECK(b.size()==a.size()); CHECK(c.size()==a.size());
    CHECK(make_instances(1,5000).size()==4096);
    CHECK(make_instances(1,0).empty());
    if(!a.empty() && a.size()==b.size() && a.size()==c.size()) {
        bool different=false;
        for(std::size_t i=0;i<a.size();++i) {
            CHECK(a[i].transform==b[i].transform);
            CHECK(a[i].color_mix==b[i].color_mix);
            different |= a[i].transform!=c[i].transform;
            for(float value:a[i].transform) CHECK(std::isfinite(value));
            CHECK(a[i].transform[0]>0 && a[i].transform[5]>0 && a[i].transform[10]>0);
            CHECK(a[i].color_mix>=0 && a[i].color_mix<=1);
            CHECK(a[i].phase>=0 && a[i].phase<=1);
            // A rampa de ativação tem largura 1/64 e termina antes da densidade 1.
            CHECK(a[i].padding>=0 && a[i].padding<=1.0f-1.0f/64.0f);
        }
        CHECK(different);
    }
    auto tunnel=make_tunnel_instances(20261011,4096);
    CHECK(tunnel.size()==4096);
    CHECK(make_tunnel_instances(1,5000).size()==4096);
    CHECK(make_tunnel_instances(1,0).empty());
    auto repeated=make_tunnel_instances(20261011,4096);
    bool overhead=false,below=false,left=false,right=false;
    std::array<bool,5> families{};
    for(std::size_t i=0;i<tunnel.size();++i) {
        const auto& item=tunnel[i];
        CHECK(item.transform==repeated[i].transform);
        for(float value:item.transform) CHECK(std::isfinite(value));
        const float x=item.transform[12],y=item.transform[13]-260;
        CHECK(std::hypot(x,y)>=700);
        overhead |= y>700; below |= y<-700; left |= x<-700; right |= x>700;
        CHECK(item.padding>=0 && item.padding<=1.0f-1.0f/64.0f);
        const auto family=static_cast<int>(item.variation)-3;
        CHECK(family>=0 && family<5);
        if(family>=0 && family<5) families[family]=true;
    }
    CHECK(overhead && below && left && right);
    for(bool family:families) CHECK(family);
    for(bool landscape:{false,true}) {
        const auto generated=make_landscape_instances(20261011,4096,landscape);
        const auto again=make_landscape_instances(20261011,4096,landscape);
        CHECK(generated.size()==4096 && again.size()==generated.size());
        CHECK(make_landscape_instances(1,0,landscape).empty());
        CHECK(make_landscape_instances(1,5000,landscape).size()==4096);
        bool tall=false,suspended=false,near=false,far=false;
        for(std::size_t i=0;i<generated.size();++i) {
            const auto& item=generated[i];
            CHECK(item.transform==again[i].transform);
            for(float value:item.transform) CHECK(std::isfinite(value));
            CHECK(item.variation>=8 && item.variation<=13);
            CHECK(item.padding>=0 && item.padding<=1-1.0f/64);
            // O corredor de avanço fica livre de estruturas monumentais.
            if(item.variation==8 || item.variation==9) CHECK(std::abs(item.transform[12])>600);
            tall |= item.transform[5]>600;
            suspended |= item.variation==12 && item.transform[13]>800;
            near |= item.transform[14]<3000;
            far |= item.transform[14]>20000;
        }
        CHECK(tall && suspended && near && far);
    }
    for(const auto& mesh:{make_fan_geometry(),make_segmented_cone_geometry(),make_diamond_geometry(),make_plate_geometry()}) {
        CHECK(mesh.vertices.size()>30 && mesh.vertices.size()<65536);
        CHECK(!mesh.indices.empty() && mesh.indices.size()%3==0);
        for(auto index:mesh.indices) CHECK(index<mesh.vertices.size());
        for(const auto& v:mesh.vertices) {
            CHECK(std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z));
            CHECK(std::abs(std::hypot(v.nx,v.ny,v.nz)-1)<0.001f);
            CHECK(v.u>=0 && v.u<=1 && v.v>=0 && v.v<=1);
        }
    }
    const auto fan=make_fan_geometry();
    const auto& fan_center=fan.vertices[12*65+16];
    for(const auto offset:{1,65}) {
        const auto& before=fan.vertices[12*65+16-offset];
        const auto& after=fan.vertices[12*65+16+offset];
        const float dx=after.x-before.x,dy=after.y-before.y,dz=after.z-before.z;
        CHECK(std::abs(dx*fan_center.nx+dy*fan_center.ny+dz*fan_center.nz)/std::hypot(dx,dy,dz)<0.01f);
    }
    for(const auto& mesh:{make_sphere_geometry(),make_torus_geometry(),make_ribbon_geometry(),make_panel_geometry()}) {
        CHECK(mesh.vertices.size()>100 && mesh.vertices.size()<65536);
        CHECK(mesh.indices.size()>300 && mesh.indices.size()%3==0);
        for(auto index:mesh.indices) CHECK(index<mesh.vertices.size());
        for(const auto& vertex:mesh.vertices) {
            CHECK(std::isfinite(vertex.x) && std::isfinite(vertex.y) && std::isfinite(vertex.z));
            CHECK(std::abs(std::sqrt(vertex.nx*vertex.nx+vertex.ny*vertex.ny+vertex.nz*vertex.nz)-1)<0.001f);
            CHECK(vertex.u>=0 && vertex.u<=1 && vertex.v>=0 && vertex.v<=1);
        }
    }
    for(const auto& mesh:{make_sphere_geometry(),make_torus_geometry()}) {
        std::size_t row_start=0;
        for(std::size_t i=0;i<mesh.vertices.size();++i) if(mesh.vertices[i].u==1) {
            const auto& first=mesh.vertices[row_start]; const auto& last=mesh.vertices[i];
            CHECK(std::hypot(first.x-last.x,first.y-last.y,first.z-last.z)<0.0001f);
            row_start=i+1;
        }
    }
    const auto ring_tunnel=make_directed_tunnel_instances(20261011);
    std::size_t axial_rings{};
    for(const auto& item:ring_tunnel) if(item.variation==5) {
        ++axial_rings;
        CHECK_NEAR(item.transform[12],0,1e-9);
        CHECK_NEAR(item.transform[13],260,1e-9);
        CHECK(item.transform[0]*0.70f>600);
        CHECK(item.color_mix>=0);
    }
    CHECK(axial_rings==64);
    return test_finish();
}
