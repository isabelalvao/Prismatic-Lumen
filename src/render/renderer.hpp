#pragma once
#include "control/parameters.hpp"
#include "geometry.hpp"
#include "sequence.hpp"
#include "time_display.hpp"
#include "config/visual_config.hpp"
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
        std::function<void()> before_graphics_shutdown={},const std::filesystem::path& soundtrack={},const VisualConfig& config=default_visual_config());
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    void resize(std::uint16_t width, std::uint16_t height);
    void draw(const Parameters& state, double time_seconds, double camera_position);
    void draw_vr_eye(const Parameters& state, double time_seconds, double camera_position,
        const vr::EyeView& eye, bool mirror);
    void flush();
    void set_scene_layout(SceneLayout layout);
    void set_composition(const Composition& composition);
    void set_time_display(const TimeDisplay& display);
    void request_capture(const std::filesystem::path& path);
    bool capture_complete() const;
    std::string device_description() const;
    std::size_t opening_mesh_count() const;
    std::size_t beat_count() const;
    float beat_strength(double time) const;
    int ring_pulse_group(double time) const;
    double soundtrack_duration() const;
    std::size_t route_adjustments() const;
    void set_visual_config(const VisualConfig& config,std::optional<double> effective_time=std::nullopt);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
