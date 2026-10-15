#pragma once
#include <SDL3/SDL.h>
#include <filesystem>
#include <memory>
#include <optional>
#include "behavior/motif_curve.hpp"

namespace fantasynt {
class AudioPlayer {
public:
    explicit AudioPlayer(const std::filesystem::path& path,std::optional<double> playback_end=std::nullopt,
                         bool scheduled_motifs=false,behavior::MotifDose dose={});
    void seek(double seconds);
    void set_paused(bool paused);
    double position() const;
    double duration() const;
private:
    struct FreeBuffer { void operator()(Uint8* p) const { SDL_free(p); } };
    std::unique_ptr<Uint8,FreeBuffer> data_;
    std::unique_ptr<SDL_AudioStream,decltype(&SDL_DestroyAudioStream)> stream_{nullptr,SDL_DestroyAudioStream};
    Uint32 length_{};
    int bytes_per_frame_{},rate_{};
    bool paused_{true};
};
}
