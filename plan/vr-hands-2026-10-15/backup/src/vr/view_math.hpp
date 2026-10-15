#pragma once
#include <array>
namespace fantasynt::vr {
struct Vector3 { float x{},y{},z{}; };
struct Quaternion { float x{},y{},z{},w{1}; };
struct Pose { Quaternion orientation; Vector3 position; };
struct Fov { float left{},right{},up{},down{}; };
struct EyeMatrices {
    std::array<float,16> view{},projection{};
    Vector3 eye_world;
};
EyeMatrices make_eye_matrices(const Pose& pose,const Fov& fov,float near_plane=10,float far_plane=28000);
}
