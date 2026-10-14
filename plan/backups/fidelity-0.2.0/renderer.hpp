#pragma once
#include "control/parameters.hpp"
#include "vr/openxr_session.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>

namespace fantasynt {
class Renderer {
public:
    Renderer(void* native_window, std::uint16_t width, std::uint16_t height,
        const std::filesystem::path& shader_directory, void* graphics_device=nullptr,
        std::function<void()> before_graphics_shutdown={});
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    void resize(std::uint16_t width, std::uint16_t height);
    void draw(const Parameters& state, double time_seconds, double camera_position);
    void draw_vr_eye(const Parameters& state, double time_seconds, double camera_position,
        const vr::EyeView& eye, bool mirror);
    void flush();
    void request_capture(const std::filesystem::path& path);
    bool capture_complete() const;
    std::string device_description() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
