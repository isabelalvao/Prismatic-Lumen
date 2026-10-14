#include "renderer.hpp"
#include "capture.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "post_process.hpp"
#include <bx/math.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace fantasynt {
namespace {
struct Runtime {
    bool live{};
    std::function<void()> before_shutdown;
    ~Runtime() {
        if(live) {
            // Liberta primeiro os invólucros das imagens, depois a sessão XR.
            // O bgfx mantém a última referência do dispositivo até encerrar o backend.
            if(before_shutdown) {
                for(int i=0;i<3;++i) bgfx::frame();
                before_shutdown();
            }
            bgfx::shutdown();
        }
    }
};
bgfx::ShaderHandle load_shader(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary|std::ios::ate);
    if(!input) throw std::runtime_error("Shader file missing: "+path.string());
    const auto size=input.tellg();
    if(size<=0||size>16*1024*1024) throw std::runtime_error("Invalid shader size");
    std::vector<char> bytes(static_cast<std::size_t>(size));
    input.seekg(0); input.read(bytes.data(),size);
    if(!input) throw std::runtime_error("Cannot read shader: "+path.string());
    auto shader=bgfx::createShader(bgfx::copy(bytes.data(),static_cast<std::uint32_t>(bytes.size())));
    if(!bgfx::isValid(shader)) throw std::runtime_error("Cannot create shader: "+path.string());
    return shader;
}
bgfx::ProgramHandle program(const std::filesystem::path& directory,const char* vertex,const char* fragment) {
    auto vs=load_shader(directory/(std::string(vertex)+".bin"));
    bgfx::ShaderHandle fs=BGFX_INVALID_HANDLE;
    try { fs=load_shader(directory/(std::string(fragment)+".bin")); }
    catch(...) { bgfx::destroy(vs); throw; }
    auto result=bgfx::createProgram(vs,fs,true);
    if(!bgfx::isValid(result)) throw std::runtime_error("Cannot link graphics program");
    return result;
}
}
struct Renderer::Impl {
    CaptureCallbacks callbacks;
    Runtime runtime;
    bgfx::SwapChain swap_chain;
    PostProcess targets;
    Mesh box,ground,quad;
    std::vector<Instance> instances=make_instances(20261011,4096);
    std::array<bgfx::ProgramHandle,6> programs;
    std::array<bgfx::UniformHandle,11> uniforms;
    std::unordered_map<void*,bgfx::FrameBufferHandle> xr_targets;
    bool vr_mode{};
    std::uint16_t mirror_width{},mirror_height{};
    std::string device;
    explicit Impl(void* window,std::uint16_t width,std::uint16_t height,const std::filesystem::path& shader_dir,void* graphics_device,
        std::function<void()> before_shutdown) {
        runtime.before_shutdown=std::move(before_shutdown);
        programs.fill(BGFX_INVALID_HANDLE); uniforms.fill(BGFX_INVALID_HANDLE);
        vr_mode=graphics_device!=nullptr;
        if(vr_mode) bgfx::renderFrame();
        bgfx::Init config;
        config.type=bgfx::RendererType::Direct3D11;
        config.vendorId=vr_mode?BGFX_PCI_ID_NONE:0x10de;
        config.platformData.context=graphics_device;
        swap_chain=config.swapChain;
        swap_chain.nwh=window;
        swap_chain.width=width; swap_chain.height=height;
        config.swapChain=swap_chain;
        config.reset=vr_mode?BGFX_RESET_NONE:BGFX_RESET_VSYNC;
        config.callback=&callbacks;
        if(!bgfx::init(config)) throw std::runtime_error("Cannot initialize Direct3D 11 renderer");
        runtime.live=true;
        try {
            const auto* caps=bgfx::getCaps();
            std::ostringstream description;
            description<<bgfx::getRendererName(caps->rendererType)<<" vendor=0x"<<std::hex<<caps->vendorId<<" device=0x"<<caps->deviceId;
            device=description.str();
            programs[0]=program(shader_dir,"vs_scene","fs_scene");
            programs[1]=program(shader_dir,"vs_scene","fs_ground");
            programs[2]=program(shader_dir,"vs_quad","fs_extract");
            programs[3]=program(shader_dir,"vs_quad","fs_blur");
            programs[4]=program(shader_dir,"vs_quad","fs_composite");
            programs[5]=program(shader_dir,"vs_quad","fs_copy");
            const std::array<const char*,11> names{"u_scene","u_clock","u_color0","u_color1","s_reflection","s_source","s_bloom","u_blur","u_post","u_eye","u_sky"};
            for(std::size_t i=0;i<names.size();++i) {
                uniforms[i]=bgfx::createUniform(names[i],i==10?bgfx::UniformType::Mat4:(i>=4&&i<=6?bgfx::UniformType::Sampler:bgfx::UniformType::Vec4));
                if(!bgfx::isValid(uniforms[i])) throw std::runtime_error("Cannot create shader uniform");
            }
            box=make_box(); ground=make_ground(); quad=make_quad();
            resize(width,height);
        } catch(...) { cleanup(); throw; }
    }
    ~Impl() { cleanup(); }
    void cleanup() {
        if(vr_mode) bgfx::setViewFrameBuffer(5,BGFX_INVALID_HANDLE);
        for(auto& [texture,target]:xr_targets) bgfx::destroy(target);
        xr_targets.clear();
        targets.destroy(); box.destroy(); ground.destroy(); quad.destroy();
        for(auto& p:programs) { if(bgfx::isValid(p)) bgfx::destroy(p); p=BGFX_INVALID_HANDLE; }
        for(auto& u:uniforms) { if(bgfx::isValid(u)) bgfx::destroy(u); u=BGFX_INVALID_HANDLE; }
    }
    void resize(std::uint16_t width,std::uint16_t height) {
        mirror_width=width; mirror_height=height;
        if(width && height) {
            swap_chain.width=width; swap_chain.height=height;
            bgfx::reset(vr_mode?BGFX_RESET_NONE:BGFX_RESET_VSYNC,&swap_chain);
        }
        if(vr_mode) return;
        configure_targets(width,height);
    }
    void configure_targets(std::uint16_t width,std::uint16_t height) {
        targets.resize(width,height);
        if(!width||!height) return;
        for(std::uint16_t v=0;v<6;++v) {
            const bool half=v>=2&&v<=4;
            bgfx::setViewRect(v,0,0,half?targets.half_width:width,half?targets.half_height:height);
            bgfx::setViewMode(v,bgfx::ViewMode::Sequential);
        }
        bgfx::setViewFrameBuffer(0,targets.reflection);
        bgfx::setViewFrameBuffer(1,targets.scene);
        for(std::uint16_t v=2;v<=4;++v) bgfx::setViewFrameBuffer(v,targets.blur[v-2]);
        bgfx::setViewFrameBuffer(5,BGFX_INVALID_HANDLE);
        for(std::uint16_t v=0;v<2;++v)
            bgfx::setViewClear(v,BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH,0x000000ff,1.0f,0);
        for(std::uint16_t v=2;v<6;++v) bgfx::setViewClear(v,BGFX_CLEAR_COLOR,0x000000ff);
    }
    void scene_uniforms(const Parameters& state,double time,double camera,bool reflection) {
        const float scene[]={state.light_intensity,state.pulse_strength,state.fog_density,state.geometry_density};
        const float clock[]={static_cast<float>(time),static_cast<float>(camera),41600.0f,reflection?1.0f:0.0f};
        const float a[]={state.palette[0].r,state.palette[0].g,state.palette[0].b,1};
        const float b[]={state.palette[1].r,state.palette[1].g,state.palette[1].b,1};
        bgfx::setUniform(uniforms[0],scene); bgfx::setUniform(uniforms[1],clock);
        bgfx::setUniform(uniforms[2],a); bgfx::setUniform(uniforms[3],b);
    }
    void boxes(std::uint16_t view,const bgfx::InstanceDataBuffer& buffer) {
        bgfx::setVertexBuffer(0,box.vertices); bgfx::setIndexBuffer(box.indices);
        bgfx::setInstanceDataBuffer(&buffer);
        bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_WRITE_Z|BGFX_STATE_DEPTH_TEST_LESS);
        bgfx::submit(view,programs[0]);
    }
    void full_screen(std::uint16_t view,bgfx::ProgramHandle p,bgfx::TextureHandle texture) {
        bgfx::setVertexBuffer(0,quad.vertices); bgfx::setIndexBuffer(quad.indices);
        bgfx::setTexture(0,uniforms[5],texture);
        bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A);
        bgfx::submit(view,p);
    }
    void draw(const Parameters& state,double time,double camera) {
        if(!targets.width||!targets.height) { bgfx::frame(); return; }
        float view[16],projection[16];
        bx::mtxLookAt(view,bx::Vec3{0,260,-450},bx::Vec3{0,440,3000});
        bx::mtxProj(projection,72.0f,static_cast<float>(targets.width)/targets.height,10.0f,28000.0f,bgfx::getCaps()->homogeneousDepth);
        const float eye[]={0,260,-450,0}; bgfx::setUniform(uniforms[9],eye);
        draw_scene(state,time,camera,view,projection,false,false,BGFX_INVALID_HANDLE);
    }
    void draw_vr_eye(const Parameters& state,double time,double camera,const vr::EyeView& eye,bool mirror) {
        if(!vr_mode) throw std::runtime_error("VR renderer requires the OpenXR graphics device");
        if(targets.width!=eye.width||targets.height!=eye.height) configure_targets(eye.width,eye.height);
        auto found=xr_targets.find(eye.texture);
        if(found==xr_targets.end()) {
            auto texture=bgfx::createTexture2D(eye.width,eye.height,false,1,
                eye.bgra?bgfx::TextureFormat::BGRA8:bgfx::TextureFormat::RGBA8,
                BGFX_TEXTURE_RT|BGFX_SAMPLER_U_CLAMP|BGFX_SAMPLER_V_CLAMP|(eye.srgb?BGFX_TEXTURE_SRGB:0),
                nullptr,reinterpret_cast<std::uint64_t>(eye.texture));
            if(!bgfx::isValid(texture)) throw std::runtime_error("Cannot wrap OpenXR image");
            auto target=bgfx::createFrameBuffer(1,&texture,true);
            if(!bgfx::isValid(target)) { bgfx::destroy(texture); throw std::runtime_error("Cannot bind OpenXR image"); }
            found=xr_targets.emplace(eye.texture,target).first;
        }
        const auto& p=eye.matrices.eye_world;
        const float origin[]={p.x,p.y,p.z,0}; bgfx::setUniform(uniforms[9],origin);
        draw_scene(state,time,camera,eye.matrices.view.data(),eye.matrices.projection.data(),eye.srgb,mirror,found->second);
    }
    void draw_scene(const Parameters& state,double time,double camera,const float* view,const float* projection,
        bool srgb,bool mirror,bgfx::FrameBufferHandle output) {
        bgfx::setViewFrameBuffer(5,output);
        // Céu preso ao mundo: usa a orientação e a projeção próprias de cada olho.
        float sky[16]{};
        for(int i=0;i<3;++i) {
            sky[i]=view[i*4]/projection[0];
            sky[4+i]=-view[i*4+1]/projection[5];
            sky[8+i]=view[i*4+2]-view[i*4]*projection[8]/projection[0]-view[i*4+1]*projection[9]/projection[5];
        }
        sky[15]=1; bgfx::setUniform(uniforms[10],sky);
        for(std::uint16_t v=0;v<2;++v) { bgfx::setViewTransform(v,view,projection); bgfx::touch(v); }
        if(bgfx::getAvailInstanceDataBuffer(4096,sizeof(Instance))<4096) throw std::runtime_error("Insufficient instance buffer");
        static_assert(sizeof(Instance)==80);
        bgfx::InstanceDataBuffer data;
        bgfx::allocInstanceDataBuffer(&data,4096,sizeof(Instance));
        std::memcpy(data.data,instances.data(),instances.size()*sizeof(Instance));
        scene_uniforms(state,time,camera,true); boxes(0,data);
        scene_uniforms(state,time,camera,false);
        Instance floor;
        floor.transform={50000,0,0,0, 0,1,0,0, 0,0,50000,0, 0,0,0,1};
        floor.variation=2.0f;
        bgfx::InstanceDataBuffer plane;
        bgfx::allocInstanceDataBuffer(&plane,1,sizeof(Instance));
        std::memcpy(plane.data,&floor,sizeof(floor));
        bgfx::setVertexBuffer(0,ground.vertices); bgfx::setIndexBuffer(ground.indices);
        bgfx::setInstanceDataBuffer(&plane);
        bgfx::setTexture(0,uniforms[4],bgfx::getTexture(targets.reflection));
        bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_WRITE_Z|BGFX_STATE_DEPTH_TEST_LESS);
        bgfx::submit(1,programs[1]);
        scene_uniforms(state,time,camera,false); boxes(1,data);
        full_screen(2,programs[2],bgfx::getTexture(targets.scene));
        const float horizontal[]={1.0f/targets.half_width,0,0,0};
        bgfx::setUniform(uniforms[7],horizontal);
        full_screen(3,programs[3],bgfx::getTexture(targets.blur[0]));
        const float vertical[]={0,1.0f/targets.half_height,0,0};
        bgfx::setUniform(uniforms[7],vertical);
        full_screen(4,programs[3],bgfx::getTexture(targets.blur[1]));
        const float post[]={state.bloom,srgb?1.0f:0.0f,0,0}; bgfx::setUniform(uniforms[8],post);
        bgfx::setTexture(1,uniforms[6],bgfx::getTexture(targets.blur[2]));
        full_screen(5,programs[4],bgfx::getTexture(targets.scene));
        if(mirror&&mirror_width&&mirror_height) {
            bgfx::setViewRect(6,0,0,mirror_width,mirror_height);
            bgfx::setViewFrameBuffer(6,BGFX_INVALID_HANDLE);
            bgfx::setViewClear(6,BGFX_CLEAR_COLOR,0x000000ff);
            bgfx::setViewMode(6,bgfx::ViewMode::Sequential);
            full_screen(6,programs[5],bgfx::getTexture(output));
        }
        bgfx::frame();
    }
};
Renderer::Renderer(void* window,std::uint16_t w,std::uint16_t h,const std::filesystem::path& shader_dir,void* graphics_device,
    std::function<void()> before_shutdown)
    : impl_(std::make_unique<Impl>(window,w,h,shader_dir,graphics_device,std::move(before_shutdown))) {}
Renderer::~Renderer()=default;
void Renderer::resize(std::uint16_t w,std::uint16_t h) { impl_->resize(w,h); }
void Renderer::draw(const Parameters& state,double time,double camera) { impl_->draw(state,time,camera); }
void Renderer::draw_vr_eye(const Parameters& state,double time,double camera,const vr::EyeView& eye,bool mirror) {
    impl_->draw_vr_eye(state,time,camera,eye,mirror);
}
void Renderer::flush() { bgfx::frame(); }
void Renderer::request_capture(const std::filesystem::path& path) {
    if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
    impl_->callbacks.completed=false; impl_->callbacks.failed=false;
    const auto utf8=path.u8string();
    bgfx::requestScreenShot(BGFX_INVALID_HANDLE,reinterpret_cast<const char*>(utf8.c_str()));
}
bool Renderer::capture_complete() const {
    if(impl_->callbacks.failed) throw std::runtime_error("Failed to write screenshot");
    return impl_->callbacks.completed;
}
std::string Renderer::device_description() const { return impl_->device; }
}
