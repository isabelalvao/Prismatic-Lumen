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
    return test_finish();
}
