#include "geometry.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace fantasynt {
namespace {
constexpr float pi=std::numbers::pi_v<float>;
template<class Function>
void patch(SurfaceGeometry& mesh,int columns,int rows,Function vertex) {
    const auto offset=static_cast<std::uint16_t>(mesh.vertices.size());
    for(int y=0;y<=rows;++y) for(int x=0;x<=columns;++x)
        mesh.vertices.push_back(vertex(static_cast<float>(x)/columns,static_cast<float>(y)/rows));
    for(int y=0;y<rows;++y) for(int x=0;x<columns;++x) {
        const auto a=static_cast<std::uint16_t>(offset+y*(columns+1)+x);
        const auto b=static_cast<std::uint16_t>(a+columns+1);
        mesh.indices.insert(mesh.indices.end(),{a,static_cast<std::uint16_t>(a+1),b,
            static_cast<std::uint16_t>(a+1),static_cast<std::uint16_t>(b+1),b});
    }
}
}
SurfaceGeometry make_fan_geometry() {
    SurfaceGeometry mesh;
    auto position=[](float u,float v) {
        const float horizontal=u*2-1;
        const float height=std::pow(1-std::abs(horizontal),0.65f);
        return SurfaceVertex{horizontal*(0.05f+0.95f*v),2*v*height,
            0.26f*std::sin(pi*v)*std::abs(horizontal),0,0,-1,u,v};
    };
    patch(mesh,64,24,[&](float u,float v) {
        auto vertex=position(u,v);
        // Diferenças centrais calculam as tangentes; nos limites usam um só lado.
        const auto left=position(std::max(0.0f,u-0.0001f),v),right=position(std::min(1.0f,u+0.0001f),v);
        const auto down=position(u,std::max(0.0f,v-0.0001f)),up=position(u,std::min(1.0f,v+0.0001f));
        const float ax=right.x-left.x,ay=right.y-left.y,az=right.z-left.z;
        const float bx=up.x-down.x,by=up.y-down.y,bz=up.z-down.z;
        const float nx=az*by-ay*bz,ny=ax*bz-az*bx,nz=ay*bx-ax*by;
        const float length=std::hypot(nx,ny,nz);
        if(length>1e-12f) { vertex.nx=nx/length; vertex.ny=ny/length; vertex.nz=nz/length; }
        return vertex;
    });
    return mesh;
}
SurfaceGeometry make_segmented_cone_geometry() {
    SurfaceGeometry mesh;
    for(int segment=0;segment<7;++segment) {
        const float bottom=segment*2.0f/7,top=(segment+0.73f)*2.0f/7;
        patch(mesh,48,1,[bottom,top](float u,float v) {
            const float angle=u*2*pi,y=bottom+(top-bottom)*v,r=1-y*0.5f;
            const float c=std::cos(angle),s=std::sin(angle);
            return SurfaceVertex{r*c,y,r*s,0.894427f*c,0.447214f,0.894427f*s,u,y*0.5f};
        });
        for(int cap=0;cap<2;++cap) {
            const float y=cap?top:bottom;
            patch(mesh,48,1,[y,cap](float u,float v) {
                const float a=u*2*pi,r=(1-y*0.5f)*v;
                return SurfaceVertex{r*std::cos(a),y,r*std::sin(a),0,cap?1.0f:-1.0f,0,u,v};
            });
        }
    }
    return mesh;
}
SurfaceGeometry make_diamond_geometry() {
    SurfaceGeometry mesh;
    for(int half=0;half<2;++half) patch(mesh,8,1,[half](float u,float v) {
        const float a=u*2*pi,y=half?v:-v,r=1-v;
        const float c=std::cos(a),s=std::sin(a);
        return SurfaceVertex{r*c,y,r*s,0.707107f*c,half?0.707107f:-0.707107f,0.707107f*s,u,v};
    });
    return mesh;
}
SurfaceGeometry make_plate_geometry() {
    SurfaceGeometry mesh;
    patch(mesh,8,1,[](float u,float v) {
        const float a=u*2*pi,c=std::cos(a),s=std::sin(a);
        return SurfaceVertex{c,v*2-1,s,c,0,s,u,v};
    });
    for(int cap=0;cap<2;++cap) patch(mesh,8,1,[cap](float u,float v) {
        const float a=u*2*pi;
        return SurfaceVertex{v*std::cos(a),cap?1.0f:-1.0f,v*std::sin(a),0,cap?1.0f:-1.0f,0,u,v};
    });
    return mesh;
}
}
