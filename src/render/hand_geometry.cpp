#include "hand_geometry.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace fantasynt {
namespace {
using vr::Vector3;
Vector3 add(Vector3 a,Vector3 b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
Vector3 scale(Vector3 a,float s) { return {a.x*s,a.y*s,a.z*s}; }
Vector3 cross(Vector3 a,Vector3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float length(Vector3 a) { return std::hypot(a.x,a.y,a.z); }
constexpr int sides=8,rings=6;
constexpr float tau=2*std::numbers::pi_v<float>;
// Índices de XR_HAND_JOINT_SET_DEFAULT_EXT: palma, pulso, polegar e quatro dedos.
constexpr std::array<std::array<std::size_t,2>,25> bones{{
    {0,1},{1,2},{2,3},{3,4},{4,5},
    {0,6},{6,7},{7,8},{8,9},{9,10},
    {0,11},{11,12},{12,13},{13,14},{14,15},
    {0,16},{16,17},{17,18},{18,19},{19,20},
    {0,21},{21,22},{22,23},{23,24},{24,25}
}};
void vertex(HandGeometry& mesh,Vector3 position,Vector3 normal) {
    mesh.vertices.push_back({position.x,position.y,position.z,normal.x,normal.y,normal.z});
}
void triangle(HandGeometry& mesh,std::size_t a,std::size_t b,std::size_t c) {
    for(auto i:{a,b,c}) mesh.indices.push_back(static_cast<std::uint16_t>(i));
}
void sphere(HandGeometry& mesh,Vector3 center,float radius) {
    const auto base=mesh.vertices.size();
    for(int row=0;row<=rings;++row) {
        const float angle=std::numbers::pi_v<float>*row/rings;
        for(int side=0;side<sides;++side) {
            const float azimuth=tau*side/sides;
            const Vector3 normal{std::sin(angle)*std::cos(azimuth),std::cos(angle),std::sin(angle)*std::sin(azimuth)};
            vertex(mesh,add(center,scale(normal,radius)),normal);
        }
    }
    for(int row=0;row<rings;++row) for(int side=0;side<sides;++side) {
        const auto a=base+row*sides+side,b=base+row*sides+(side+1)%sides;
        if(row>0) triangle(mesh,a,b,a+sides);
        if(row<rings-1) triangle(mesh,b,b+sides,a+sides);
    }
}
void bone(HandGeometry& mesh,Vector3 a,Vector3 b,float ra,float rb) {
    const Vector3 delta{b.x-a.x,b.y-a.y,b.z-a.z};
    const float size=length(delta);
    // Articulações coincidentes continuam a ser esferas; não formar bases degeneradas.
    if(!std::isfinite(size)||size<0.001f) return;
    const auto axis=scale(delta,1/size);
    auto side=cross(axis,std::abs(axis.y)<0.9f?Vector3{0,1,0}:Vector3{1,0,0});
    side=scale(side,1/length(side));
    const auto up=cross(axis,side);
    const auto base=mesh.vertices.size();
    for(int i=0;i<sides;++i) {
        const float angle=tau*i/sides;
        const auto radial=add(scale(side,std::cos(angle)),scale(up,std::sin(angle)));
        auto normal=add(radial,scale(axis,(ra-rb)/size));
        normal=scale(normal,1/length(normal));
        vertex(mesh,add(a,scale(radial,ra)),normal);
        vertex(mesh,add(b,scale(radial,rb)),normal);
    }
    for(int i=0;i<sides;++i) {
        const auto a0=base+2*i,b0=base+2*((i+1)%sides);
        triangle(mesh,a0,a0+1,b0); triangle(mesh,b0,a0+1,b0+1);
    }
    ++mesh.bone_count;
}
}
HandGeometry make_hand_geometry(const interaction::HandSample& hand,float height,float back) {
    HandGeometry mesh;
    if(!hand.active||hand.locate_result<0||!std::isfinite(height)||!std::isfinite(back)) return mesh;
    mesh.vertices.reserve(26*(rings+1)*sides+25*2*sides);
    mesh.indices.reserve(26*6*(rings-1)*sides+25*6*sides);
    std::array<bool,interaction::hand_joint_count> usable{};
    std::array<Vector3,interaction::hand_joint_count> points{};
    std::array<float,interaction::hand_joint_count> radii{};
    for(std::size_t i=0;i<hand.joints.size();++i) {
        const auto& joint=hand.joints[i];
        if(!joint.position_valid||!joint.position_tracked) continue;
        const auto& p=joint.pose.position;
        if(!std::isfinite(p.x)||!std::isfinite(p.y)||!std::isfinite(p.z)) continue;
        points[i]=vr::tracking_to_world(p,height,back);
        if(!std::isfinite(points[i].x)||!std::isfinite(points[i].y)||!std::isfinite(points[i].z)) continue;
        usable[i]=true;
        // Raio apenas visual: limitar valores extremos, sem alterar aquisição ou descritores.
        radii[i]=std::isfinite(joint.radius_m)&&joint.radius_m>0?std::clamp(joint.radius_m,0.001f,0.025f)*100:0.6f;
        sphere(mesh,points[i],radii[i]); ++mesh.joint_count;
    }
    for(const auto& pair:bones) {
        const auto a=pair[0],b=pair[1];
        if(usable[a]&&usable[b]) bone(mesh,points[a],points[b],radii[a]*0.65f,radii[b]*0.65f);
    }
    return mesh;
}
}
