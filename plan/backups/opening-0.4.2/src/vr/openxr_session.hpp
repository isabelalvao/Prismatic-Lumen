#pragma once
#include "view_math.hpp"
#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
namespace fantasynt::vr {
struct EyeView {
    EyeMatrices matrices;
    void* texture{};
    std::uint16_t width{},height{};
    bool bgra{},srgb{};
};
std::string probe_openxr();
class OpenXRSession {
public:
    explicit OpenXRSession(float resolution_scale=0.7f);
    ~OpenXRSession();
    OpenXRSession(const OpenXRSession&)=delete;
    OpenXRSession& operator=(const OpenXRSession&)=delete;
    void* graphics_device() const;
    std::string description() const;
    bool poll_events();
    std::optional<std::array<EyeView,2>> begin_frame();
    void end_frame();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
