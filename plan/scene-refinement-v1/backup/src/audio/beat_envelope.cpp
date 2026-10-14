#include "beat_envelope.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace fantasynt {
BeatEnvelope BeatEnvelope::analyze(std::span<const std::int16_t> pcm,int rate,int channels,const VisualConfig& config) {
    if(rate<8000||rate>192000||rate%100||channels<1||channels>2||pcm.size()%channels||pcm.size()>128000000)
        throw std::runtime_error("Audio: invalid beat analysis input");
    BeatEnvelope result;
    const auto frames=pcm.size()/channels;
    const auto hop=static_cast<std::size_t>(rate/100);
    std::vector<float> energy;
    energy.reserve(frames/hop+1);
    const double low=1-std::exp(-2*3.141592653589793*config[Setting::BassHigh]/rate);
    const double dc=1-std::exp(-2*3.141592653589793*config[Setting::BassLow]/rate);
    double bass1{},bass2{},baseline{},power{};
    std::size_t count{};
    for(std::size_t frame=0;frame<frames;++frame) {
        double value{};
        for(int channel=0;channel<channels;++channel) value+=pcm[frame*channels+channel]/32768.0;
        value/=channels;
        // Banda grave 35–120 Hz; dois polos rejeitam transientes agudos.
        baseline+=dc*(value-baseline);
        bass1+=low*((value-baseline)-bass1);
        bass2+=low*(bass1-bass2);
        power+=bass2*bass2;
        if(++count==hop||frame+1==frames) {
            energy.push_back(static_cast<float>(std::sqrt(power/count)));
            count=0; power=0;
        }
    }
    result.envelope_.resize(energy.size()+1);
    if(energy.empty()) return result;
    const float peak=*std::max_element(energy.begin(),energy.end());
    const float minimum=std::max(static_cast<float>(config[Setting::PulseMinimum]),peak*static_cast<float>(config[Setting::PulseRelative]));
    std::vector<float> novelty(energy.size());
    for(std::size_t i=0;i<energy.size();++i) novelty[i]=std::max(0.0f,energy[i]-(i>=3?energy[i-3]:0));
    double last=-1;
    for(std::size_t i=1;i+2<energy.size();++i) {
        float background{};
        const auto begin=i>80?i-80:0;
        const auto end=i>15?i-15:0;
        for(std::size_t j=begin;j<end;++j) background+=energy[j];
        if(end>begin) background/=static_cast<float>(end-begin);
        const float flux=novelty[i];
        const double time=static_cast<double>(i)/envelope_rate;
        if(time-last<config[Setting::PulseInterval]||flux<minimum||energy[i]<background*config[Setting::PulseBackground]||
            flux<novelty[i-1]||flux<novelty[i+1]||flux<novelty[i+2]) continue;
        last=time;
        // Compensa a janela de energia, sem deslocar as amostras do áudio.
        const double onset=std::max(0.0,time-config[Setting::PulseCompensation]);
        result.onsets_.push_back(onset);
        const float strength=std::clamp(energy[i]/std::max(peak*static_cast<float>(config[Setting::PulseReference]),minimum),0.25f,1.0f);
        const auto first=static_cast<std::size_t>(std::floor(onset*envelope_rate));
        for(std::size_t j=first;j<result.envelope_.size()&&j<=first+static_cast<std::size_t>(std::ceil(config[Setting::PulseTail]*envelope_rate));++j) {
            const double dt=static_cast<double>(j)/envelope_rate-onset;
            if(dt<0) continue;
            const float shape=static_cast<float>(std::min(1.0,dt/config[Setting::PulseAttack])*std::exp(-std::max(0.0,dt-config[Setting::PulseAttack])/config[Setting::PulseDecay]));
            const float tail=static_cast<float>(std::clamp((config[Setting::PulseTail]-dt)/std::min(0.08,config[Setting::PulseTail]*0.25),0.0,1.0));
            result.envelope_[j]=std::max(result.envelope_[j],strength*shape*tail);
        }
    }
    return result;
}
float BeatEnvelope::sample(double seconds) const {
    if(!std::isfinite(seconds)||seconds<0||envelope_.empty()) return 0;
    const double frame=seconds*envelope_rate;
    if(frame>=envelope_.size()-1) return 0;
    const auto left=static_cast<std::size_t>(frame);
    const float fraction=static_cast<float>(frame-left);
    return envelope_[left]+(envelope_[left+1]-envelope_[left])*fraction;
}
}
