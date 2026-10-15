#pragma once
#include <cstdint>
#include <span>
#include "behavior/motif_curve.hpp"

namespace fantasynt {
// PCM16 estéreo, amostras no relógio original; nunca reamostra nem muda o ficheiro.
void apply_master_envelope(std::span<std::int16_t> stereo,int rate,std::uint64_t first_frame=0,
                           double attenuation=behavior::gain_depth);
}
