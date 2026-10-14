#pragma once
#include <bgfx/bgfx.h>
#include <array>

namespace fantasynt {
class PostProcess {
public:
    PostProcess() = default;
    ~PostProcess();
    void resize(std::uint16_t width, std::uint16_t height);
    void destroy();
    bgfx::FrameBufferHandle reflection{BGFX_INVALID_HANDLE};
    bgfx::FrameBufferHandle scene{BGFX_INVALID_HANDLE};
    std::array<bgfx::FrameBufferHandle, 3> blur{{BGFX_INVALID_HANDLE,BGFX_INVALID_HANDLE,BGFX_INVALID_HANDLE}};
    std::uint16_t width{},height{},half_width{},half_height{};
};
}
