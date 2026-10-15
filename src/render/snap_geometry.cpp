#include "snap_geometry.hpp"
#include <cmath>
#include <numbers>

namespace fantasynt {
float snap_particle_fade(const interaction::SnapEvent& event,std::int64_t now) {
    const double age=static_cast<double>(now-event.xr_time_ns)/1e9;
    if(age<0||age>=snap_particle_lifetime) return 0;
    const float remaining=static_cast<float>(1-age/snap_particle_lifetime);
    return remaining*remaining;
}
HandGeometry make_snap_geometry(const interaction::SnapEvent& event,std::int64_t now,float height,float back) {
    HandGeometry mesh;
    const float fade=snap_particle_fade(event,now);
    if(fade<=0||!std::isfinite(height)||!std::isfinite(back)) return mesh;
    const auto p=event.position_m;
    if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)) return mesh;
    const float t=static_cast<float>((now-event.xr_time_ns)/1e9);
    constexpr int count=14;
    constexpr std::array<vr::Vector3,6> axes{{{1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}}};
    constexpr std::array<std::uint16_t,24> faces{0,2,4,2,1,4,1,3,4,3,0,4,2,0,5,1,2,5,3,1,5,0,3,5};
    mesh.vertices.reserve(count*6); mesh.indices.reserve(count*24);
    for(int i=0;i<count;++i) {
        const float z=1-2*(i+0.5f)/count,r=std::sqrt(1-z*z);
        const float angle=i*std::numbers::pi_v<float>*(3-std::sqrt(5.0f))+static_cast<float>(event.hand)*0.7f;
        const float travel=snap_spread_scale*(0.012f+(0.18f+0.025f*(i%5))*t);
        auto local=p;
        local.x+=r*std::cos(angle)*travel; local.y+=z*travel-0.12f*t*t; local.z+=r*std::sin(angle)*travel;
        const auto center=vr::tracking_to_world(local,height,back);
        const float radius=snap_point_scale*(0.0015f+0.0003f*(i%3))*100*std::sqrt(fade);
        const auto base=static_cast<std::uint16_t>(mesh.vertices.size());
        for(const auto a:axes) mesh.vertices.push_back({center.x+a.x*radius,center.y+a.y*radius,center.z+a.z*radius,a.x,a.y,a.z});
        for(const auto index:faces) mesh.indices.push_back(static_cast<std::uint16_t>(base+index));
    }
    return mesh;
}
}
