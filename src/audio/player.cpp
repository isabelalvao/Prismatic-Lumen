#include "player.hpp"
#include "master_envelope.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>
#include <limits>

namespace fantasynt {
namespace {
void checked(bool ok) {
    if(!ok) throw std::runtime_error(std::string("Audio: ")+SDL_GetError());
}
}
AudioPlayer::AudioPlayer(const std::filesystem::path& path,std::optional<double> playback_end,
                         bool scheduled_motifs,behavior::MotifDose dose) {
    checked(SDL_InitSubSystem(SDL_INIT_AUDIO));
    const auto encoded=path.u8string();
    SDL_AudioSpec spec{};
    Uint8* buffer{};
    checked(SDL_LoadWAV(reinterpret_cast<const char*>(encoded.c_str()),&spec,&buffer,&length_));
    data_.reset(buffer);
    if(spec.format!=SDL_AUDIO_S16LE || spec.channels!=2 || spec.freq!=44100 || !length_ || length_%4 ||
       length_>static_cast<Uint32>(std::numeric_limits<int>::max()))
        throw std::runtime_error("Audio: expected stereo PCM16 44100 Hz WAV");
    bytes_per_frame_=4; rate_=spec.freq;
    if(duration()<playback_end.value_or(424)) throw std::runtime_error("Audio: soundtrack must cover selected duration");
    if(playback_end) {
        if(!std::isfinite(*playback_end)||*playback_end<=0||*playback_end>duration())
            throw std::runtime_error("Audio: playback end must be within soundtrack duration");
        // Limita a fila de amostras: a saída nunca consome o segmento seguinte.
        length_=static_cast<Uint32>(std::floor(*playback_end*rate_))*bytes_per_frame_;
    }
    // Guião P1 imutável: ganho por frame PCM, calculado uma vez antes da fila SDL.
    // Pause/seek/restart continuam a usar exatamente os mesmos índices de frames.
    if(scheduled_motifs) {
        dose=behavior::validated_dose(dose);
        apply_master_envelope({reinterpret_cast<std::int16_t*>(data_.get()),length_/2},rate_,0,dose.master_attenuation);
    }
    stream_.reset(SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,&spec,nullptr,nullptr));
    checked(stream_!=nullptr);
    seek(0);
}
double AudioPlayer::duration() const { return static_cast<double>(length_/bytes_per_frame_)/rate_; }
double AudioPlayer::position() const {
    const auto queued=SDL_GetAudioStreamQueued(stream_.get());
    if(queued<0) throw std::runtime_error(std::string("Audio: ")+SDL_GetError());
    if(static_cast<Uint32>(queued)>length_) throw std::runtime_error("Audio: invalid queue length");
    // Relógio dos frames consumidos pelo dispositivo SDL; não inclui latência física.
    return static_cast<double>((length_-static_cast<Uint32>(queued))/bytes_per_frame_)/rate_;
}
void AudioPlayer::set_paused(bool paused) {
    if(paused==paused_) return;
    checked(paused?SDL_PauseAudioStreamDevice(stream_.get()):SDL_ResumeAudioStreamDevice(stream_.get()));
    paused_=paused;
}
void AudioPlayer::seek(double seconds) {
    if(!std::isfinite(seconds)) throw std::runtime_error("Audio: seek must be finite");
    set_paused(true);
    checked(SDL_ClearAudioStream(stream_.get()));
    const auto frame=static_cast<Uint32>(std::floor(std::clamp(seconds,0.0,duration())*rate_));
    const auto offset=std::min(frame*static_cast<Uint32>(bytes_per_frame_),length_);
    if(offset<length_) checked(SDL_PutAudioStreamData(stream_.get(),data_.get()+offset,static_cast<int>(length_-offset)));
    checked(SDL_FlushAudioStream(stream_.get()));
}
}
