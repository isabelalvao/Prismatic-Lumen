#pragma once
#include "art_direction.hpp"
#include <array>
namespace fantasynt {
using VisualUniforms=std::array<std::array<float,4>,22>;
VisualUniforms visual_uniforms(const VisualConfig& config,const ArtMotion& motion,double time=0,bool extension=true);
}
