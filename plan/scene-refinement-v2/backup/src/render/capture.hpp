#pragma once
#include <bgfx/bgfx.h>
#include <bimg/bimg.h>
#include <bx/error.h>
#include <bx/file.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>

namespace fantasynt {
class CaptureCallbacks final : public bgfx::CallbackI {
public:
    std::atomic<bool> completed{false};
    std::atomic<bool> failed{false};
    void fatal(const char*, std::uint16_t, bgfx::Fatal::Enum code, const char* message) override {
        std::fprintf(stderr, "Graphics error: %s\n", message);
        if (code != bgfx::Fatal::DebugCheck) std::abort();
    }
    void traceVargs(const char*, std::uint16_t, const char* format, va_list args) override {
        if (std::getenv("FANTASYNT_GRAPHICS_TRACE")) std::vfprintf(stderr, format, args);
    }
    void profilerBegin(const char*, std::uint32_t, const char*, std::uint16_t) override {}
    void profilerBeginLiteral(const char*, std::uint32_t, const char*, std::uint16_t) override {}
    void profilerEnd() override {}
    std::uint32_t cacheReadSize(std::uint64_t) override { return 0; }
    bool cacheRead(std::uint64_t, void*, std::uint32_t) override { return false; }
    void cacheWrite(std::uint64_t, const void*, std::uint32_t) override {}
    void screenShot(const char* path, std::uint32_t width, std::uint32_t height,
        std::uint32_t pitch, bgfx::TextureFormat::Enum format, const void* data,
        std::uint32_t, bool yflip) override {
        bx::FileWriter writer;
        bx::Error error;
        if (!bx::open(&writer, path, false, &error)) {
            failed = true; completed = true; return;
        }
        const auto pixel_format = format == bgfx::TextureFormat::RGBA8 ?
            bimg::TextureFormat::RGBA8 : bimg::TextureFormat::BGRA8;
        bimg::imageWritePng(&writer, width, height, pitch, data, pixel_format, yflip, &error);
        bx::close(&writer);
        failed = !error.isOk();
        completed = true;
    }
    void captureBegin(std::uint32_t, std::uint32_t, std::uint32_t, bgfx::TextureFormat::Enum, bool) override {}
    void captureEnd() override {}
    void captureFrame(const void*, std::uint32_t) override {}
};
}
