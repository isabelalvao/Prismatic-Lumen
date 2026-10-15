#include "master_envelope.hpp"
#include "behavior/motif_curve.hpp"
#include <cmath>
#include <stdexcept>

namespace fantasynt {
void apply_master_envelope(std::span<std::int16_t> stereo,int rate,std::uint64_t first_frame) {
    if(rate<=0||stereo.size()%2) throw std::invalid_argument("Invalid stereo envelope block");
    for(std::size_t i=0;i<stereo.size()/2;++i) {
        const double gain=behavior::master_gain(static_cast<double>(first_frame+i)/rate);
        if(gain==1.0) continue; // Mantém PCM original fora dos motivos, incluindo o final.
        for(std::size_t channel=0;channel<2;++channel)
            stereo[2*i+channel]=static_cast<std::int16_t>(std::lround(stereo[2*i+channel]*gain));
    }
}
}
