#include "vr/view_math.hpp"
#include "test_support.hpp"
#include <cmath>
using namespace fantasynt::vr;
std::array<float,4> transform(const std::array<float,16>& m,float x,float y,float z) {
    return {m[0]*x+m[4]*y+m[8]*z+m[12],m[1]*x+m[5]*y+m[9]*z+m[13],
        m[2]*x+m[6]*y+m[10]*z+m[14],m[3]*x+m[7]*y+m[11]*z+m[15]};
}
int main() {
    const Fov fov{-0.7f,0.9f,0.8f,-0.6f};
    Pose pose; auto center=make_eye_matrices(pose,fov);
    CHECK_NEAR(center.eye_world.y,160,0.001f);
    CHECK_NEAR(center.eye_world.z,-450,0.001f);
    auto origin=transform(center.view,0,160,-450);
    CHECK_NEAR(origin[0],0,0.001f); CHECK_NEAR(origin[1],0,0.001f); CHECK_NEAR(origin[2],0,0.001f);
    auto ahead=transform(center.view,0,160,-350);
    CHECK_NEAR(ahead[2],100,0.001f);
    auto left=pose,right=pose;
    left.position.x=-0.032f; right.position.x=0.032f;
    auto l=make_eye_matrices(left,fov),r=make_eye_matrices(right,fov);
    CHECK_NEAR(r.eye_world.x-l.eye_world.x,6.4f,0.001f);
    CHECK(l.view!=r.view);
    pose.position={0.5f,0.2f,-0.7f};
    auto translated=make_eye_matrices(pose,fov);
    CHECK_NEAR(translated.eye_world.x,50,0.001f);
    CHECK_NEAR(translated.eye_world.y,180,0.001f);
    CHECK_NEAR(translated.eye_world.z,-380,0.001f);
    pose.position={}; pose.orientation={0,std::sqrt(0.5f),0,std::sqrt(0.5f)};
    auto rotated=make_eye_matrices(pose,fov);
    auto yaw=transform(rotated.view,-100,160,-450);
    CHECK_NEAR(yaw[0],0,0.001f); CHECK_NEAR(yaw[2],100,0.001f);
    const auto& p=center.projection;
    auto near=transform(p,0,0,10),far=transform(p,0,0,28000);
    CHECK_NEAR(near[2]/near[3],0,0.0001f); CHECK_NEAR(far[2]/far[3],1,0.0001f);
    auto edge_l=transform(p,std::tan(fov.left)*100,0,100);
    auto edge_r=transform(p,std::tan(fov.right)*100,0,100);
    auto edge_u=transform(p,0,std::tan(fov.up)*100,100);
    auto edge_d=transform(p,0,std::tan(fov.down)*100,100);
    CHECK_NEAR(edge_l[0]/edge_l[3],-1,0.0001f); CHECK_NEAR(edge_r[0]/edge_r[3],1,0.0001f);
    CHECK_NEAR(edge_u[1]/edge_u[3],1,0.0001f); CHECK_NEAR(edge_d[1]/edge_d[3],-1,0.0001f);
    return test_finish();
}
