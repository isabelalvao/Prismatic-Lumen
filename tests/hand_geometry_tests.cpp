#include "render/hand_geometry.hpp"
#include "test_support.hpp"
#include <limits>
using namespace fantasynt;
using interaction::HandSample;

HandSample sample() {
    HandSample hand; hand.active=true; hand.status="located";
    for(std::size_t i=0;i<hand.joints.size();++i) {
        auto& joint=hand.joints[i];
        joint.position_valid=joint.position_tracked=true;
        joint.pose.position={0.2f+static_cast<float>(i%5)*0.01f,static_cast<float>(i)*0.01f,-0.4f};
        joint.radius_m=0.005f;
    }
    return hand;
}
bool finite_mesh(const HandGeometry& mesh) {
    for(const auto& v:mesh.vertices) {
        if(!std::isfinite(v.x)||!std::isfinite(v.y)||!std::isfinite(v.z)||
            !std::isfinite(v.nx)||!std::isfinite(v.ny)||!std::isfinite(v.nz)) return false;
        const float length=std::hypot(v.nx,v.ny,v.nz);
        if(std::abs(length-1)>0.0001f) return false;
    }
    for(auto i:mesh.indices) if(i>=mesh.vertices.size()) return false;
    return mesh.vertices.size()<65536&&mesh.indices.size()%3==0;
}
int main() {
    auto hand=sample(); const auto mesh=make_hand_geometry(hand);
    CHECK(mesh.joint_count==26); CHECK(mesh.bone_count==25); CHECK(finite_mesh(mesh));
    // Polo superior da primeira esfera: metros -> cm, Y de base, inversão de Z.
    CHECK_NEAR(mesh.vertices[0].x,20,0.0001f);
    CHECK_NEAR(mesh.vertices[0].y,160.5f,0.0001f);
    CHECK_NEAR(mesh.vertices[0].z,-410,0.0001f);
    const auto shifted=make_hand_geometry(hand,260,350);
    for(std::size_t i=0;i<mesh.vertices.size();++i) {
        CHECK_NEAR(shifted.vertices[i].x,mesh.vertices[i].x,0.0001f);
        CHECK_NEAR(shifted.vertices[i].y-mesh.vertices[i].y,100,0.0001f);
        CHECK_NEAR(shifted.vertices[i].z-mesh.vertices[i].z,100,0.0001f);
    }
    // A orientação não é necessária para desenhar segmentos entre posições.
    hand.joints[8].orientation_valid=hand.joints[8].orientation_tracked=false;
    CHECK(make_hand_geometry(hand).joint_count==26);
    hand.joints[8].position_valid=false;
    auto partial=make_hand_geometry(hand);
    CHECK(partial.joint_count==25); CHECK(partial.bone_count==23); CHECK(finite_mesh(partial));
    hand=sample(); hand.joints[8].position_tracked=false;
    CHECK(make_hand_geometry(hand).joint_count==25); CHECK(make_hand_geometry(hand).bone_count==23);
    hand.active=false;
    CHECK(make_hand_geometry(hand).vertices.empty());
    hand=sample(); hand.locate_result=-1;
    CHECK(make_hand_geometry(hand).vertices.empty());
    hand=sample(); hand.joints[8].pose.position.x=std::numeric_limits<float>::quiet_NaN();
    partial=make_hand_geometry(hand);
    CHECK(partial.joint_count==25); CHECK(partial.bone_count==23); CHECK(finite_mesh(partial));
    hand=sample();
    for(auto& joint:hand.joints) { joint.pose.position={0,0,-0.4f}; joint.radius_m=std::numeric_limits<float>::infinity(); }
    const auto coincident=make_hand_geometry(hand);
    CHECK(coincident.joint_count==26); CHECK(coincident.bone_count==0); CHECK(finite_mesh(coincident));
    // Sem memória de poses: perda e recuperação, incluindo mudança da origem LOCAL.
    hand=sample(); hand.active=false;
    CHECK(make_hand_geometry(hand).indices.empty());
    hand=sample(); const auto before=make_hand_geometry(hand);
    for(auto& joint:hand.joints) joint.pose.position.x+=1;
    const auto after=make_hand_geometry(hand);
    CHECK_NEAR(after.vertices[0].x-before.vertices[0].x,100,0.0001f);
    const vr::Fov fov{-0.9f,0.9f,0.7f,-0.7f};
    const auto eye_point=[](const vr::EyeMatrices& eye,const HandVertex& v) {
        const auto& m=eye.view;
        return vr::Vector3{m[0]*v.x+m[4]*v.y+m[8]*v.z+m[12],
            m[1]*v.x+m[5]*v.y+m[9]*v.z+m[13],m[2]*v.x+m[6]*v.y+m[10]*v.z+m[14]};
    };
    vr::Pose left,right; left.position.x=-0.032f; right.position.x=0.032f;
    const auto l=eye_point(vr::make_eye_matrices(left,fov),before.vertices[0]);
    const auto r=eye_point(vr::make_eye_matrices(right,fov),before.vertices[0]);
    CHECK_NEAR(l.x-r.x,6.4f,0.0001f); CHECK_NEAR(l.z,40,0.0001f);
    left.position.x+=1;
    const auto recentered=eye_point(vr::make_eye_matrices(left,fov),after.vertices[0]);
    CHECK_NEAR(recentered.x,l.x,0.0001f); CHECK_NEAR(recentered.y,l.y,0.0001f); CHECK_NEAR(recentered.z,l.z,0.0001f);
    return test_finish();
}
