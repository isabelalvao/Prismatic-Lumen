#include "post_process.hpp"
#include <algorithm>
#include <stdexcept>

namespace fantasynt {
namespace {
bgfx::FrameBufferHandle target(std::uint16_t w,std::uint16_t h,bool depth) {
    const auto flags=BGFX_TEXTURE_RT|BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP;
    auto color=bgfx::createTexture2D(w,h,false,1,bgfx::TextureFormat::RGBA16F,flags);
    if(!bgfx::isValid(color)) throw std::runtime_error("Cannot create HDR render texture");
    if(!depth) {
        auto result=bgfx::createFrameBuffer(1,&color,true);
        if(!bgfx::isValid(result)) { bgfx::destroy(color); throw std::runtime_error("Cannot create framebuffer"); }
        return result;
    }
    auto z=bgfx::createTexture2D(w,h,false,1,bgfx::TextureFormat::D24S8,BGFX_TEXTURE_RT);
    if(!bgfx::isValid(z)) { bgfx::destroy(color); throw std::runtime_error("Cannot create depth texture"); }
    bgfx::TextureHandle textures[2]={color,z};
    auto result=bgfx::createFrameBuffer(2,textures,true);
    if(!bgfx::isValid(result)) { bgfx::destroy(color); bgfx::destroy(z); throw std::runtime_error("Cannot create depth framebuffer"); }
    return result;
}
}
PostProcess::~PostProcess() { destroy(); }
void PostProcess::destroy() {
    if(bgfx::isValid(reflection)) bgfx::destroy(reflection);
    if(bgfx::isValid(scene)) bgfx::destroy(scene);
    reflection=scene=BGFX_INVALID_HANDLE;
    for(auto& handle:blur) {
        if(bgfx::isValid(handle)) bgfx::destroy(handle);
        handle=BGFX_INVALID_HANDLE;
    }
}
void PostProcess::resize(std::uint16_t w,std::uint16_t h) {
    destroy(); width=w; height=h;
    if(!w || !h) return;
    half_width=std::max<std::uint16_t>(1,w/2);
    half_height=std::max<std::uint16_t>(1,h/2);
    reflection=target(w,h,true);
    scene=target(w,h,true);
    for(auto& handle:blur) handle=target(half_width,half_height,false);
}
}
