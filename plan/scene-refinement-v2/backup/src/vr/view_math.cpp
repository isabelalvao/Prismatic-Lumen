#include "view_math.hpp"
#include <cmath>
#include <stdexcept>
namespace fantasynt::vr {
namespace {
Vector3 cross(Vector3 a,Vector3 b) { return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x}; }
float dot(Vector3 a,Vector3 b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
Vector3 normalize(Vector3 v) {
    const float length=std::sqrt(dot(v,v));
    if(!std::isfinite(length)||length<0.00001f) throw std::runtime_error("Invalid XR orientation");
    return {v.x/length,v.y/length,v.z/length};
}
Vector3 rotate(Quaternion q,Vector3 v) {
    auto a=cross({q.x,q.y,q.z},v);
    auto b=cross({q.x,q.y,q.z},a);
    return {v.x+2*(q.w*a.x+b.x),v.y+2*(q.w*a.y+b.y),v.z+2*(q.w*a.z+b.z)};
}
}
EyeMatrices make_eye_matrices(const Pose& pose,const Fov& fov,float near_plane,float far_plane) {
    const float left=std::tan(fov.left),right=std::tan(fov.right),up=std::tan(fov.up),down=std::tan(fov.down);
    if(!std::isfinite(left+right+up+down)||right<=left||up<=down||near_plane<=0||far_plane<=near_plane)
        throw std::runtime_error("Invalid XR projection");
    EyeMatrices result;
    // OpenXR usa metros e -Z para a frente; a cena usa centímetros e +Z.
    result.eye_world={pose.position.x*100,160+pose.position.y*100,-450-pose.position.z*100};
    auto forward=rotate(pose.orientation,{0,0,-1}); forward.z=-forward.z; forward=normalize(forward);
    auto up_vector=rotate(pose.orientation,{0,1,0}); up_vector.z=-up_vector.z;
    auto side=normalize(cross(up_vector,forward));
    auto vertical=cross(forward,side);
    result.view={side.x,vertical.x,forward.x,0, side.y,vertical.y,forward.y,0,
        side.z,vertical.z,forward.z,0, -dot(side,result.eye_world),-dot(vertical,result.eye_world),-dot(forward,result.eye_world),1};
    result.projection={2/(right-left),0,0,0, 0,2/(up-down),0,0,
        -(right+left)/(right-left),-(up+down)/(up-down),far_plane/(far_plane-near_plane),1,
        0,0,-far_plane*near_plane/(far_plane-near_plane),0};
    return result;
}
}
