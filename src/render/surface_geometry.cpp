#include "geometry.hpp"
#include <cmath>
#include <numbers>

namespace fantasynt {
namespace {
template<class Function>
SurfaceGeometry surface(int columns,int rows,Function vertex) {
    SurfaceGeometry mesh;
    for(int y=0;y<=rows;++y) for(int x=0;x<=columns;++x)
        mesh.vertices.push_back(vertex(static_cast<float>(x)/columns,static_cast<float>(y)/rows));
    for(int y=0;y<rows;++y) for(int x=0;x<columns;++x) {
        const auto a=static_cast<std::uint16_t>(y*(columns+1)+x);
        const auto b=static_cast<std::uint16_t>(a+columns+1);
        mesh.indices.insert(mesh.indices.end(),{a,static_cast<std::uint16_t>(a+1),b,
            static_cast<std::uint16_t>(a+1),static_cast<std::uint16_t>(b+1),b});
    }
    return mesh;
}
constexpr float pi=std::numbers::pi_v<float>;
}
SurfaceGeometry make_sphere_geometry() {
    return surface(32,16,[](float u,float v) {
        const float theta=u*2*pi,phi=v*pi;
        const float x=std::sin(phi)*std::cos(theta),y=std::cos(phi),z=std::sin(phi)*std::sin(theta);
        return SurfaceVertex{x,y,z,x,y,z,u,v};
    });
}
SurfaceGeometry make_torus_geometry() {
    return surface(96,8,[](float u,float v) {
        const float theta=u*2*pi,phi=v*2*pi,c=std::cos(theta),s=std::sin(theta);
        const float radius=0.82f+0.12f*std::cos(phi);
        return SurfaceVertex{c*radius,s*radius,0.12f*std::sin(phi),
            c*std::cos(phi),s*std::cos(phi),std::sin(phi),u,v};
    });
}
SurfaceGeometry make_ribbon_geometry() {
    return surface(40,6,[](float u,float v) {
        const float theta=(u-0.5f)*pi*1.6f,radius=0.60f+v*0.35f;
        const float c=std::cos(theta),s=std::sin(theta);
        const float z=0.20f*std::sin(theta*2);
        const float dx=-s*radius,dy=c*radius,dz=0.40f*std::cos(theta*2);
        const float nx=-dz*s,ny=dz*c,nz=-radius;
        const float inverse=1/std::sqrt(nx*nx+ny*ny+nz*nz);
        return SurfaceVertex{c*radius,s*radius,z,nx*inverse,ny*inverse,nz*inverse,u,v};
    });
}
SurfaceGeometry make_panel_geometry() {
    SurfaceGeometry mesh;
    for(int face=0;face<6;++face) {
        const auto offset=static_cast<std::uint16_t>(mesh.vertices.size());
        auto patch=surface(4,4,[face](float u,float v) {
            const float a=u*2-1,b=v*2-1;
            const float sign=face%2?1.0f:-1.0f;
            const float p[]={face/2==0?sign:a,face/2==1?sign:(face/2==0?a:b),face/2==2?sign:b};
            float base[3],normal[3];
            float length{};
            for(int i=0;i<3;++i) { base[i]=std::fmin(0.82f,std::fmax(-0.82f,p[i])); normal[i]=p[i]-base[i]; length+=normal[i]*normal[i]; }
            const float inverse=1/std::sqrt(length);
            for(float& value:normal) value*=inverse;
            return SurfaceVertex{base[0]+normal[0]*0.18f,base[1]+normal[1]*0.18f,base[2]+normal[2]*0.18f,
                normal[0],normal[1],normal[2],u,v};
        });
        mesh.vertices.insert(mesh.vertices.end(),patch.vertices.begin(),patch.vertices.end());
        for(auto index:patch.indices) mesh.indices.push_back(static_cast<std::uint16_t>(index+offset));
    }
    return mesh;
}
}
