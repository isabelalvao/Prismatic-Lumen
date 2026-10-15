#include "pencil_geometry.hpp"
#include <cmath>
#include <numbers>

namespace fantasynt {
namespace {
using vr::Vector3;
Vector3 sub(Vector3 a,Vector3 b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
Vector3 cross(Vector3 a,Vector3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float length(Vector3 a) { return std::hypot(a.x,a.y,a.z); }
Vector3 normalized(Vector3 a) { const float n=length(a); return {a.x/n,a.y/n,a.z/n}; }
float fade(const interaction::PencilPoint& p,std::int64_t now) {
    const double age=static_cast<double>(now-p.xr_time_ns)/1e9;
    return age<0||age>=interaction::pencil_lifetime?0:static_cast<float>(1-age/interaction::pencil_lifetime);
}
void vertex(PencilGeometry& mesh,Vector3 p,Vector3 n,float f) { mesh.vertices.push_back({p.x,p.y,p.z,n.x,n.y,n.z,f,0}); }
bool finite(Vector3 p) { return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z); }
}
PencilGeometry make_pencil_geometry(std::span<const interaction::PencilPoint> points,std::int64_t now,float height,float back) {
    PencilGeometry mesh;
    if(!std::isfinite(height)||!std::isfinite(back)) return mesh;
    if(points.size()>interaction::pencil_max_points) points=points.last(interaction::pencil_max_points);
    mesh.vertices.reserve(points.size()*18); mesh.indices.reserve(points.size()*60);
    constexpr std::array<Vector3,6> axes{{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}};
    constexpr std::array<std::uint16_t,24> faces{0,2,4,2,1,4,1,3,4,3,0,4,2,0,5,1,2,5,3,1,5,0,3,5};
    for(std::size_t i=0;i<points.size();++i) {
        const auto& point=points[i]; const float f=fade(point,now);
        if(f<=0||!finite(point.position_m)) continue;
        const auto center=vr::tracking_to_world(point.position_m,height,back);
        if(!finite(center)) continue;
        const float radius=0.12f*(0.30f+0.70f*f); // 1,2 mm no traço novo.
        const auto base=static_cast<std::uint16_t>(mesh.vertices.size());
        const float bead=i+1==points.size()?radius*1.4f:radius;
        for(const auto n:axes) vertex(mesh,{center.x+n.x*bead,center.y+n.y*bead,center.z+n.z*bead},n,f);
        for(const auto index:faces) mesh.indices.push_back(static_cast<std::uint16_t>(base+index));
        if(!i||points[i-1].stroke!=point.stroke||fade(points[i-1],now)<=0||!finite(points[i-1].position_m)) continue;
        const auto start=vr::tracking_to_world(points[i-1].position_m,height,back);
        const auto delta=sub(center,start); const float distance=length(delta);
        if(!finite(start)||!std::isfinite(distance)||distance<0.01f||distance>12.0f) continue;
        const auto direction=normalized(delta);
        const auto side=normalized(cross(direction,std::abs(direction.y)<0.9f?Vector3{0,1,0}:Vector3{1,0,0}));
        const auto up=cross(direction,side);
        const auto ring=static_cast<std::uint16_t>(mesh.vertices.size());
        const float old_fade=fade(points[i-1],now),old_radius=0.12f*(0.30f+0.70f*old_fade);
        for(int k=0;k<6;++k) {
            const float angle=k*std::numbers::pi_v<float>/3;
            const Vector3 n{side.x*std::cos(angle)+up.x*std::sin(angle),side.y*std::cos(angle)+up.y*std::sin(angle),side.z*std::cos(angle)+up.z*std::sin(angle)};
            vertex(mesh,{start.x+n.x*old_radius,start.y+n.y*old_radius,start.z+n.z*old_radius},n,old_fade);
            vertex(mesh,{center.x+n.x*radius,center.y+n.y*radius,center.z+n.z*radius},n,f);
        }
        for(std::uint16_t k=0;k<6;++k) {
            const auto a=static_cast<std::uint16_t>(ring+2*k),b=static_cast<std::uint16_t>(ring+2*((k+1)%6));
            for(auto index:{a,static_cast<std::uint16_t>(a+1),b,b,static_cast<std::uint16_t>(a+1),static_cast<std::uint16_t>(b+1)})
                mesh.indices.push_back(index);
        }
    }
    return mesh;
}
}
