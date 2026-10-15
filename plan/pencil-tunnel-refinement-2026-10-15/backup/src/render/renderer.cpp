#include "renderer.hpp"
#include "capture.hpp"
#include "geometry.hpp"
#include "mesh.hpp"
#include "post_process.hpp"
#include "opening_scene.hpp"
#include "art_direction.hpp"
#include "mini_poles.hpp"
#include "surface_choreography.hpp"
#include "spectral_geometry.hpp"
#include "rotation_history.hpp"
#include "audio/beat_envelope.hpp"
#include "visual_uniforms.hpp"
#include "hand_geometry.hpp"
#include "snap_geometry.hpp"
#include "pencil_geometry.hpp"
#include <SDL3/SDL.h>
#include <bimg/decode.h>
#include <bx/allocator.h>
#include <bx/math.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <cmath>

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
    if(!bgfx::isValid(result)) throw std::runtime_error("Cannot link graphics program: "+std::string(vertex)+" / "+fragment);
    return result;
}
bgfx::TextureHandle detail_texture(const std::filesystem::path& path) {
    std::ifstream f(path,std::ios::binary|std::ios::ate);
    if(!f||f.tellg()<=0||f.tellg()>16*1024*1024) throw std::runtime_error("Opening texture missing or invalid: "+path.string());
    std::vector<char> data(static_cast<std::size_t>(f.tellg())); f.seekg(0); f.read(data.data(),data.size());
    if(!f) throw std::runtime_error("Cannot read opening texture");
    bx::DefaultAllocator allocator;
    auto* image=bimg::imageParse(&allocator,data.data(),static_cast<std::uint32_t>(data.size()),bimg::TextureFormat::RGBA8);
    if(!image) throw std::runtime_error("Cannot decode opening texture");
    if(image->m_width==0||image->m_height==0||image->m_width>2048||image->m_height>2048) {
        bimg::imageFree(image);
        throw std::runtime_error("Opening texture dimensions exceed limit");
    }
    const auto texture=bgfx::createTexture2D(static_cast<std::uint16_t>(image->m_width),static_cast<std::uint16_t>(image->m_height),false,1,
        bgfx::TextureFormat::RGBA8,0,bgfx::copy(image->m_data,image->m_size));
    bimg::imageFree(image);
    if(!bgfx::isValid(texture)) throw std::runtime_error("Cannot create opening texture");
    return texture;
}
BeatEnvelope load_beats(const std::filesystem::path& path,const VisualConfig& config) {
    std::error_code error;
    const auto size=std::filesystem::file_size(path,error);
    if(error||size>256*1024*1024) throw std::runtime_error("Audio: beat soundtrack missing or oversized");
    SDL_AudioSpec spec{}; Uint8* buffer{}; Uint32 length{};
    const auto encoded=path.u8string();
    if(!SDL_LoadWAV(reinterpret_cast<const char*>(encoded.c_str()),&spec,&buffer,&length))
        throw std::runtime_error(std::string("Audio: ")+SDL_GetError());
    const std::unique_ptr<Uint8,decltype(&SDL_free)> data(buffer,SDL_free);
    if(spec.format!=SDL_AUDIO_S16LE||spec.channels!=2||spec.freq!=44100||length%4)
        throw std::runtime_error("Audio: expected stereo PCM16 44100 Hz WAV for beats");
    return BeatEnvelope::analyze(std::span<const std::int16_t>(reinterpret_cast<const std::int16_t*>(buffer),length/2),spec.freq,spec.channels,config);
}
}
struct Renderer::Impl {
    CaptureCallbacks callbacks;
    Runtime runtime;
    bgfx::SwapChain swap_chain;
    PostProcess targets;
    Mesh box,ground,quad,sphere,torus,ribbon,panel,diamond,plate;
    std::array<Mesh,spectral_cone_piece_count> luminous_cone;
    std::array<std::array<Mesh,crystal_piece_count>,crystal_variant_count> crystals;
    bool legacy_field{};
    Composition composition;
    std::array<std::vector<Instance>,3> scenes{{make_landscape_instances(20261011,4096,false),
        make_landscape_instances(20261011,4096,true),make_tunnel_instances(20261011,4096)}};
    std::array<std::vector<Instance>,2> extension_scenes{{scenes[0],scenes[1]}};
    std::vector<Instance> legacy_instances=make_instances(20261011,4096);
    std::array<bgfx::ProgramHandle,9> programs;
    std::array<bgfx::UniformHandle,18> uniforms;
    bgfx::UniformHandle timer_uniform=BGFX_INVALID_HANDLE;
    bgfx::UniformHandle spectral_uniform=BGFX_INVALID_HANDLE;
    bgfx::UniformHandle hand_colour=BGFX_INVALID_HANDLE;
    std::array<HandGeometry,2> hand_geometry;
    interaction::SnapDetector snap_detector;
    std::array<std::optional<interaction::SnapEvent>,2> snap_bursts;
    std::array<HandGeometry,2> snap_geometry;
    std::array<float,2> snap_fade{};
    std::optional<std::int64_t> snap_time;
    std::uint64_t snap_epoch{};
    interaction::LightPencil light_pencil;
    std::array<PencilGeometry,2> pencil_geometry;
    TimeDisplay time_display;
    std::optional<TimeDisplayUniforms> stereo_timer;
    VisualConfig visual;
    ArtMotion phase_offset;
    double last_time{};
    double clock_offset{};
    std::vector<AngularRateKey> cone_rates;
    std::vector<Instance> directed_tunnel=scenes[2];
    std::optional<double> ring_alignment_camera;
    double ring_alignment_speed=1;
    double ring_formation_speed=1.5;
    double ring_alignment_lead=40;
    std::array<float,3> eye_origin{{0,260,-450}};
    std::array<float,16> eye_view{};
    struct TransparentDraw {
        std::uint16_t view{};
        const Mesh* mesh{};
        const OpeningGroup* group{};
        Instance instance;
        float weight{},kind{},presence{};
        double depth{};
    };
    std::vector<TransparentDraw> transparent_draws;
    std::filesystem::path beat_source;
    OpeningScene original_opening;
    BeatEnvelope beats;
    std::size_t adjusted_instances{};
    OpeningScene opening;
    std::unordered_map<std::string,Mesh> opening_meshes;
    std::unordered_map<std::string,float> opening_radii;
    std::array<bgfx::TextureHandle,3> opening_details{{BGFX_INVALID_HANDLE,BGFX_INVALID_HANDLE,BGFX_INVALID_HANDLE}};
    std::unordered_map<void*,bgfx::FrameBufferHandle> xr_targets;
    bool vr_mode{};
    std::uint16_t mirror_width{},mirror_height{};
    std::string device;
    explicit Impl(void* window,std::uint16_t width,std::uint16_t height,const std::filesystem::path& shader_dir,void* graphics_device,
        std::function<void()> before_shutdown,const std::filesystem::path& soundtrack,const VisualConfig& settings) {
        visual=settings;
        cone_rates.push_back({0,0,visual[Setting::ConeRotation]*visual[Setting::AnimationSpeed]*3.141592653589793/180});
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
            programs[6]=program(shader_dir,"vs_opening","fs_opening");
            programs[7]=program(shader_dir,"vs_hand","fs_hand");
            programs[8]=program(shader_dir,"vs_light_pencil","fs_light_pencil");
            hand_colour=bgfx::createUniform("u_hand_colour",bgfx::UniformType::Vec4);
            if(!bgfx::isValid(hand_colour)) throw std::runtime_error("Cannot create hand colour uniform");
            const std::array<const char*,18> names{"u_scene","u_clock","u_color0","u_color1","s_reflection","s_source","s_bloom","u_blur","u_post","u_eye","u_sky","u_style","u_atmosphere","s_detail","u_art","u_tuning","u_layout","u_choreography"};
            for(std::size_t i=0;i<names.size();++i) {
                uniforms[i]=bgfx::createUniform(names[i],i==10?bgfx::UniformType::Mat4:((i>=4&&i<=6)||i==13?bgfx::UniformType::Sampler:bgfx::UniformType::Vec4),i==15?22:i==17?7:1);
                if(!bgfx::isValid(uniforms[i])) throw std::runtime_error("Cannot create shader uniform");
            }
            timer_uniform=bgfx::createUniform("u_timer",bgfx::UniformType::Vec4,5);
            if(!bgfx::isValid(timer_uniform)) throw std::runtime_error("Cannot create timer shader uniform");
            spectral_uniform=bgfx::createUniform("u_spectral",bgfx::UniformType::Vec4,11);
            if(!bgfx::isValid(spectral_uniform)) throw std::runtime_error("Cannot create spectral shader uniform");
            box=make_box(); ground=make_ground(); quad=make_quad();
            sphere=make_surface_mesh(make_sphere_geometry());
            torus=make_surface_mesh(make_torus_geometry());
            ribbon=make_surface_mesh(make_ribbon_geometry());
            panel=make_surface_mesh(make_panel_geometry());
            for(int piece=0;piece<spectral_cone_piece_count;++piece)
                luminous_cone[piece]=make_surface_mesh(make_luminous_cone_piece_geometry(piece));
            for(int variant=0;variant<crystal_variant_count;++variant) for(int piece=0;piece<crystal_piece_count;++piece)
                crystals[variant][piece]=make_surface_mesh(make_crystal_piece_geometry(variant,piece));
            diamond=make_surface_mesh(make_diamond_geometry());
            plate=make_surface_mesh(make_plate_geometry());
            const auto assets=std::filesystem::u8path(SDL_GetBasePath())/"opening";
            original_opening=load_opening_scene(assets);
            prepare_mini_poles(original_opening);
            opening=original_opening;
            extend_opening_intro(opening,visual);
            adjusted_instances=clear_directed_route(opening,visual);
            prepare_extensions(visual);
            prepare_tunnel_order(visual);
            beat_source=soundtrack.empty()?assets.parent_path()/"audio/ChezNous_MASTER.wav":soundtrack;
            beats=load_beats(beat_source,visual);
            for(const auto& [id,geometry]:opening.meshes) {
                opening_meshes.emplace(id,make_surface_mesh(geometry));
                float radius{};
                for(const auto& v:geometry.vertices) radius=std::max(radius,std::hypot(v.x,v.y,v.z));
                opening_radii.emplace(id,radius);
            }
            const auto textures=assets/"textures";
            opening_details[0]=detail_texture(textures/"_HelloEnjoy_/Materials/Textures/Circle_256_8.png");
            opening_details[1]=detail_texture(textures/"_HelloEnjoy_/ChezNous/Materials/Visual/Patterns/hex_hollow_pattern_256_2.png");
            opening_details[2]=detail_texture(textures/"_HelloEnjoy_/ChezNous/Materials/Visual/Patterns/hex_pattern_128.png");
            resize(width,height);
        } catch(...) { cleanup(); throw; }
    }
    ~Impl() { cleanup(); }
    ArtMotion motion(double time) const {
        auto m=sample_art_motion(time,visual);
        m.points+=phase_offset.points; m.traces+=phase_offset.traces;
        m.pyramids+=phase_offset.pyramids; m.sky+=phase_offset.sky; m.poles+=phase_offset.poles;
        m.pole_phase_base+=phase_offset.pole_phase_base; m.pole_phase_seed+=phase_offset.pole_phase_seed;
        return m;
    }
    void prepare_tunnel_order(const VisualConfig& c) {
        directed_tunnel=make_directed_tunnel_instances(20261011);
        const auto reference=directed_camera_position(c[Setting::TunnelStart],c);
        for(auto& item:directed_tunnel) {
            if(item.variation==5) continue;
            double z=std::fmod(item.transform[14]-reference+41600+1600,41600);
            if(z<0) z+=41600;
            const double angle=(std::atan2(item.transform[13]-260,item.transform[12])/6.283185307179586+0.5);
            // Anéis da proximidade para a profundidade, com peças sucessivas à volta de cada anel.
            item.phase=static_cast<float>(std::clamp((std::floor(z/650)+angle)/64,0.0,1.0));
        }
    }
    void prepare_extensions(const VisualConfig& c) {
        extension_scenes={make_landscape_instances(20261011,4096,false,c),make_landscape_instances(20261011,4096,true,c)};
        for(auto& items:extension_scenes) {
            for(auto& item:items) if(item.variation==8) {
                // A espessura dos cristais é fixa e entra no cálculo do corredor.
                const float x=std::hypot(item.transform[0],item.transform[2]),z=std::hypot(item.transform[8],item.transform[10]);
                if(z>0) { item.transform[8]*=x/z; item.transform[10]*=x/z; }
            }
            clear_extension_route(items,c);
        }
        separate_ground_rocks(extension_scenes[0],extension_scenes[1],c);
    }
    void set_config(const VisualConfig& next,double effective_time) {
        if(!std::isfinite(effective_time)||effective_time<0) throw std::runtime_error("Visual config: invalid effective time");
        const Setting layout[]={Setting::WorldScale,Setting::OffsetX,Setting::OffsetY,Setting::OffsetZ,Setting::IntroSectors,Setting::SectorSpacing,
            Setting::CorridorWidth,Setting::CorridorMargin,Setting::CorridorFloor,Setting::CorridorCeiling,Setting::PoleAmplitude,Setting::MiniPoleMinimum};
        const Setting audio[]={Setting::BassLow,Setting::BassHigh,Setting::PulseAttack,Setting::PulseDecay,Setting::PulseTail,
            Setting::PulseInterval,Setting::PulseRelative,Setting::PulseMinimum,Setting::PulseBackground,Setting::PulseReference,Setting::PulseCompensation};
        const auto changed=[&](auto& list) { return std::any_of(std::begin(list),std::end(list),[&](auto key) { return visual[key]!=next[key]; }); };
        std::optional<OpeningScene> prepared;
        std::size_t count=adjusted_instances;
        if(changed(layout)) { prepared=original_opening; extend_opening_intro(*prepared,next); count=clear_directed_route(*prepared,next); }
        std::optional<BeatEnvelope> analyzed;
        if(changed(audio)) analyzed=load_beats(beat_source,next);
        const auto before=motion(last_time),after=sample_art_motion(effective_time,next);
        const auto rotation=rotation_integral(cone_rates,last_time);
        clock_offset+=last_time*visual[Setting::AnimationSpeed]-effective_time*next[Setting::AnimationSpeed];
        if(visual[Setting::ConeRotation]!=next[Setting::ConeRotation]||visual[Setting::AnimationSpeed]!=next[Setting::AnimationSpeed]||effective_time!=last_time) {
            while(!cone_rates.empty()&&cone_rates.back().time>=effective_time) cone_rates.pop_back();
            cone_rates.push_back({effective_time,rotation,next[Setting::ConeRotation]*next[Setting::AnimationSpeed]*3.141592653589793/180});
        }
        const auto variation=next[Setting::PoleVariation]-visual[Setting::PoleVariation];
        phase_offset={before.points-after.points,before.traces-after.traces,before.pyramids-after.pyramids,before.sky-after.sky,before.poles-after.poles,
            before.pole_phase_base+before.poles*variation,before.pole_phase_seed-2*before.poles*variation};
        if(prepared) {
            opening=std::move(*prepared); adjusted_instances=count;
            prepare_extensions(next);
        }
        const Setting extension_layout[]={Setting::CrystalHeightMin,Setting::CrystalHeightMax,Setting::ConeDensityMultiplier,Setting::ConeGap};
        if(!prepared&&changed(extension_layout)) prepare_extensions(next);
        if(analyzed) beats=std::move(*analyzed);
        const Setting tunnel_layout[]={Setting::TunnelStart,Setting::RingAlignStart,Setting::RingAlignDuration,
            Setting::CameraSpeed,Setting::CameraDelay,Setting::CameraAcceleration,Setting::CameraStartZ,Setting::WorldScale,Setting::OffsetZ};
        if(changed(tunnel_layout)) prepare_tunnel_order(next);
        // A velocidade afeta os próximos passos da câmara, sem reescrever a posição já formada.
        if(visual[Setting::RingAlignStart]!=next[Setting::RingAlignStart]) ring_alignment_camera.reset();
        if(ring_alignment_camera&&(visual[Setting::RingLead]!=next[Setting::RingLead]||visual[Setting::RingAlignDuration]!=next[Setting::RingAlignDuration]||
            visual[Setting::RingSpacing]!=next[Setting::RingSpacing]))
            ring_alignment_lead=required_ring_lead(ring_formation_speed,next);
        visual=next;
        last_time=effective_time;
    }
    void cleanup() {
        if(bgfx::isValid(hand_colour)) bgfx::destroy(hand_colour);
        hand_colour=BGFX_INVALID_HANDLE;
        if(bgfx::isValid(timer_uniform)) bgfx::destroy(timer_uniform);
        timer_uniform=BGFX_INVALID_HANDLE;
        if(bgfx::isValid(spectral_uniform)) bgfx::destroy(spectral_uniform);
        spectral_uniform=BGFX_INVALID_HANDLE;
        if(vr_mode) bgfx::setViewFrameBuffer(5,BGFX_INVALID_HANDLE);
        for(auto& [texture,target]:xr_targets) bgfx::destroy(target);
        xr_targets.clear();
        targets.destroy(); box.destroy(); ground.destroy(); quad.destroy();
        sphere.destroy(); torus.destroy(); ribbon.destroy(); panel.destroy();
        diamond.destroy(); plate.destroy();
        for(auto& piece:luminous_cone) piece.destroy();
        for(auto& variant:crystals) for(auto& piece:variant) piece.destroy();
        for(auto& [id,mesh]:opening_meshes) mesh.destroy();
        for(auto texture:opening_details) if(bgfx::isValid(texture)) bgfx::destroy(texture);
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
    void scene_uniforms(const Parameters& state,double time,double camera,bool reflection,float weight=1,float kind=0) {
        const float scene[]={state.light_intensity,state.pulse_strength,state.fog_density,state.geometry_density};
        const float clock[]={static_cast<float>(time*visual[Setting::AnimationSpeed]+clock_offset),static_cast<float>(camera),41600.0f,reflection?1.0f:0.0f};
        const float a[]={state.palette[0].r,state.palette[0].g,state.palette[0].b,1};
        const float b[]={state.palette[1].r,state.palette[1].g,state.palette[1].b,1};
        bgfx::setUniform(uniforms[0],scene); bgfx::setUniform(uniforms[1],clock);
        bgfx::setUniform(uniforms[2],a); bgfx::setUniform(uniforms[3],b);
        const float style[]={weight,kind,composition.directed_extension?1.0f:0.0f,0}; bgfx::setUniform(uniforms[11],style);
        const auto& fog=composition.atmosphere;
        const float atmosphere[]={fog.r,fog.g,fog.b,composition.exposure};
        bgfx::setUniform(uniforms[12],atmosphere);
        const float art[]={beats.sample(time+visual[Setting::PulseOffset]),1,static_cast<float>(time),composition.opening_weight};
        bgfx::setUniform(uniforms[14],art);
        const auto tuning=visual_uniforms(visual,motion(time),time,composition.directed_extension);
        bgfx::setUniform(uniforms[15],tuning.data(),22);
        const float layout[]={composition.geometry_density_multiplier,25,composition.previous_exit_progress,0};
        bgfx::setUniform(uniforms[16],layout);
        const auto glow=visual.colour(ConfigColour::TunnelGlow);
        const int pulse_group=beats.pulse_group(time+visual[Setting::PulseOffset],visual[Setting::RingPulseStart]+visual[Setting::PulseOffset]);
        const float choreography[7][4]={
            {static_cast<float>(time),static_cast<float>(visual[Setting::ExtensionStart]),static_cast<float>(visual[Setting::ConeSegmentInterval]),static_cast<float>(visual[Setting::ConeSegmentReveal])},
            {0,0,static_cast<float>(visual[Setting::ConeSeedDelay]),0},
            {composition.previous_exit_progress,static_cast<float>(visual[Setting::TunnelStart]),static_cast<float>(visual[Setting::TunnelFade]),static_cast<float>(visual[Setting::TunnelPieceReveal])},
            {composition.tunnel_light_weight,composition.fog_multiplier,static_cast<float>(composition.directed_extension?visual[Setting::LedDensityScale]:1),static_cast<float>(visual[Setting::GenericPieceReveal])},
            {static_cast<float>(visual[Setting::ExtensionModulesStart]),static_cast<float>(visual[Setting::TunnelDepartureStart]),
                static_cast<float>(visual[Setting::TunnelDepartureDuration]),static_cast<float>(visual[Setting::TunnelDepartureDistance]*100)},
            {glow.r,glow.g,glow.b,static_cast<float>(visual[Setting::TunnelGlowEmission])},
            {static_cast<float>(pulse_group),beats.sample(time+visual[Setting::PulseOffset]),pulse_group>=0?1.0f:0.0f,static_cast<float>(visual[Setting::ConePaletteVariants])}};
        bgfx::setUniform(uniforms[17],choreography,7);
        std::array<std::array<float,4>,11> spectral{};
        const ConfigColour colours[]={ConfigColour::ConeCyan,ConfigColour::ConeMagenta,ConfigColour::CrystalCyan,ConfigColour::CrystalViolet};
        for(int i=0;i<4;++i) { const auto color=visual.colour(colours[i]); spectral[i]={color.r,color.g,color.b,1}; }
        spectral[4]={static_cast<float>(visual[Setting::ConeEmission]),static_cast<float>(visual[Setting::CrystalEmission]),
            static_cast<float>(visual[Setting::CrystalBody]),static_cast<float>(visual[Setting::CrystalEdge])};
        const ConfigColour extra[]={ConfigColour::RingEven,ConfigColour::RingOdd,ConfigColour::ConeBlue,ConfigColour::ConeWhite,ConfigColour::ConeViolet,ConfigColour::ConeGold};
        for(int i=0;i<6;++i) { const auto colour=visual.colour(extra[i]); spectral[i+5]={colour.r,colour.g,colour.b,1}; }
        spectral[5][3]=static_cast<float>(visual[Setting::RingPulseGain]);
        spectral[6][3]=static_cast<float>(visual[Setting::RingIdleEmission]);
        bgfx::setUniform(spectral_uniform,spectral.data(),11);
    }
    double surface_opacity(const Instance& item,const Parameters& state,double time,float weight,float kind) const {
        double density=state.geometry_density;
        if(composition.directed_extension&&item.variation>2.5f&&item.variation<7.5f&&item.variation!=5)
            density*=visual[Setting::ArtifactDensity];
        double opacity=weight*std::clamp((density-item.padding)*64.0,0.0,1.0);
        if(!composition.directed_extension) return opacity;
        SurfaceChoreographySettings settings;
        settings.tube_start=visual[Setting::TunnelStart]; settings.tube_build_seconds=visual[Setting::TunnelFade];
        settings.tube_piece_reveal_seconds=visual[Setting::TunnelPieceReveal];
        settings.generic_forms_reveal_seconds=visual[Setting::GenericPieceReveal];
        settings.segment_reveal_seconds=visual[Setting::ConeSegmentReveal];
        if(item.variation>7.5f) {
            opacity*=previous_piece_visibility(item.padding,composition.previous_exit_progress);
            if(item.variation!=8 && item.variation!=9)
                opacity*=generic_piece_visibility(item.phase,time,kind<0.5f?visual[Setting::ExtensionModulesStart]:visual[Setting::ExtensionStart],settings);
        }
        if(item.variation>2.5f && item.variation<7.5f) {
            opacity*=tube_piece_visibility(item.phase,time,composition.previous_exit_progress,settings);
            if(item.variation!=5) {
                const auto progress=tunnel_departure_progress(item.padding,time,visual[Setting::TunnelDepartureStart],visual[Setting::TunnelDepartureDuration]);
                const auto x=std::clamp((progress-0.65)/0.35,0.0,1.0);
                opacity*=1-x*x*(3-2*x);
            }
        }
        return opacity;
    }
    double draw_depth(const Instance& item,double camera,bool reflection,bool wrapped) const {
        auto x=item.transform[12],y=item.transform[13];
        double z=item.transform[14]-(wrapped&&composition.directed_extension?0:camera);
        if(wrapped&&!composition.directed_extension) {
            const bool tunnel=item.variation>2.5f&&item.variation<7.5f;
            z=std::fmod(z+41600+(tunnel?1600:0),41600);
            if(z<0) z+=41600;
            z-=tunnel?1600:0;
        }
        if(reflection) y=-y;
        return eye_view[2]*x+eye_view[6]*y+eye_view[10]*z+eye_view[14];
    }
    void surface_batch(std::uint16_t view,const Mesh& mesh,std::vector<Instance> items,
        const Parameters& state,double time,double camera,bool reflection,float weight,float kind) {
        std::vector<Instance> opaque;
        for(const auto& item:items) if(item.transform[7]>0) {
            if(item.transform[7]<0.99999f)
                transparent_draws.push_back({view,&mesh,nullptr,item,weight,kind,0,draw_depth(item,camera,reflection,true)});
            else opaque.push_back(item);
        }
        for(std::size_t offset=0;offset<opaque.size();offset+=2048) {
            const auto count=static_cast<std::uint32_t>(std::min<std::size_t>(2048,opaque.size()-offset));
            if(bgfx::getAvailInstanceDataBuffer(count,sizeof(Instance))<count) throw std::runtime_error("Surface instance buffer exhausted");
            bgfx::InstanceDataBuffer data; bgfx::allocInstanceDataBuffer(&data,count,sizeof(Instance));
            std::memcpy(data.data,opaque.data()+offset,count*sizeof(Instance));
            scene_uniforms(state,time,camera,reflection,weight,kind);
            bgfx::setVertexBuffer(0,mesh.vertices); bgfx::setIndexBuffer(mesh.indices); bgfx::setInstanceDataBuffer(&data);
            bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_DEPTH_TEST_LESS|BGFX_STATE_WRITE_Z);
            bgfx::submit(view,programs[0]);
        }
    }
    void flush_transparency(const Parameters& state,double time,double camera) {
        // Ordenação global por olho e profundidade: o piso e todas as malhas opacas já estão desenhados.
        std::stable_sort(transparent_draws.begin(),transparent_draws.end(),[](const auto& a,const auto& b) {
            return a.view!=b.view?a.view<b.view:a.depth>b.depth;
        });
        for(std::size_t first=0;first<transparent_draws.size();) {
            const auto& entry=transparent_draws[first];
            std::size_t end=first+1;
            while(end<transparent_draws.size()&&end-first<2048) {
                const auto& next=transparent_draws[end];
                if(next.view!=entry.view||next.mesh!=entry.mesh||next.group!=entry.group||
                    next.weight!=entry.weight||next.kind!=entry.kind||next.presence!=entry.presence) break;
                ++end;
            }
            const auto count=static_cast<std::uint32_t>(end-first);
            if(bgfx::getAvailInstanceDataBuffer(count,sizeof(Instance))<count) throw std::runtime_error("Transparent instance buffer exhausted");
            bgfx::InstanceDataBuffer data; bgfx::allocInstanceDataBuffer(&data,count,sizeof(Instance));
            for(std::size_t i=first;i<end;++i) std::memcpy(data.data+(i-first)*sizeof(Instance),&transparent_draws[i].instance,sizeof(Instance));
            scene_uniforms(state,time,camera,entry.view==0,entry.weight,entry.kind);
            if(entry.group) {
                const auto& group=*entry.group;
                const float art[]={beats.sample(time+visual[Setting::PulseOffset]),entry.presence,static_cast<float>(time),static_cast<float>(group.role)};
                bgfx::setUniform(uniforms[14],art);
                auto tuning=visual_uniforms(visual,motion(time),time,composition.directed_extension);
                const auto color=visual.family_colour(group.role);
                tuning[20]={color.r,color.g,color.b,static_cast<float>(visual.family_emission(group.role))};
                bgfx::setUniform(uniforms[15],tuning.data(),22);
                const auto index=group.material==1?1:group.material==2?2:0;
                bgfx::setTexture(0,uniforms[13],opening_details[index]);
            }
            bgfx::setVertexBuffer(0,entry.mesh->vertices); bgfx::setIndexBuffer(entry.mesh->indices); bgfx::setInstanceDataBuffer(&data);
            bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_DEPTH_TEST_LESS|BGFX_STATE_BLEND_ALPHA);
            bgfx::submit(entry.view,entry.group?programs[6]:programs[0]);
            first=end;
        }
        transparent_draws.clear();
    }
    void boxes(std::uint16_t view,const std::vector<Instance>& instances,
        const Parameters& state,double time,double camera,bool reflection,float weight,float kind) {
        const auto prepared=[&](const Instance& source) {
            auto item=source;
            if(composition.directed_extension) {
                if(source.variation>2.5f&&source.variation<7.5f) {
                    const double alignment=ring_alignment_camera.value_or(camera+directed_camera_step(time,visual[Setting::RingAlignStart],state.motion_speed,visual));
                    item=directed_tunnel_instance(source,time,camera,alignment,visual,ring_alignment_camera?ring_alignment_speed:state.motion_speed/0.3,
                        ring_alignment_camera?ring_alignment_lead:-1,time*visual[Setting::AnimationSpeed]+clock_offset);
                } else {
                    const auto& m=source.transform;
                    const double radius=2*(std::hypot(m[0],m[1],m[2])+std::hypot(m[4],m[5],m[6])+std::hypot(m[8],m[9],m[10]));
                    item.transform[14]=static_cast<float>(retained_wrapped_depth(m[14]-camera,radius,visual));
                }
            }
            return item;
        };
        const auto opacity_for=[&](const Instance& source,const Instance& item) {
            double opacity=surface_opacity(source,state,time,weight,kind);
            if(composition.directed_extension) {
                const auto& m=item.transform;
                const double radius=2*(std::abs(m[2])+std::abs(m[6])+std::abs(m[10]));
                opacity*=rear_piece_visibility(m[14]-eye_origin[2],radius,visual);
            }
            return opacity;
        };
        for(std::size_t first=0;first<instances.size();) {
            const float family=instances[first].variation;
            std::size_t end=legacy_field?instances.size():first+1;
            if(!legacy_field) while(end<instances.size()&&instances[end].variation==family) ++end;
            if(composition.directed_extension&&((family==10&&visual[Setting::RemoveDarkTriangles])||
                (family==13&&visual[Setting::RemovePlatforms]))) { first=end; continue; }
            if(family==8 || family==9) {
                const int variants=family==8?static_cast<int>(visual[Setting::CrystalVariants]):1;
                const int pieces=family==8?crystal_piece_count:spectral_cone_piece_count;
                for(int variant=0;variant<variants;++variant) for(int piece=0;piece<pieces;++piece) {
                    std::vector<Instance> visible;
                    for(std::size_t index=first;index<end;++index) {
                        const auto& source=instances[index];
                        if(family==8 && static_cast<int>(source.color_mix*variants)%variants!=variant) continue;
                        auto positioned=prepared(source);
                        double opacity=opacity_for(source,positioned);
                        const auto start=family==9?visual[Setting::ExtensionStart]:(kind<0.5f?visual[Setting::ExtensionModulesStart]:visual[Setting::ExtensionStart]);
                        const auto interval=visual[family==9?Setting::ConeSegmentInterval:Setting::CrystalInterval];
                        const auto reveal=visual[family==9?Setting::ConeSegmentReveal:Setting::CrystalReveal];
                        const auto delay=visual[family==9?Setting::ConeSeedDelay:Setting::CrystalDelay];
                        double piece_start=start+piece*interval+source.phase*delay;
                        if(composition.directed_extension&&family==9&&visual[Setting::ConeBeatSync])
                            piece_start=beat_aligned_time(piece_start+visual[Setting::PulseOffset],beats.onsets())-visual[Setting::PulseOffset];
                        double piece_progress=1;
                        if(composition.directed_extension) {
                            piece_progress=spectral_piece_visibility(0,0,time,piece_start,interval,reveal,0);
                            opacity*=piece_progress;
                        }
                        if(opacity<=0) continue;
                        auto item=surface_fade_instance(positioned,opacity);
                        if(family==9) {
                            auto complete=composition.directed_extension?start+(pieces-1)*interval+source.phase*delay:0;
                            if(composition.directed_extension&&visual[Setting::ConeBeatSync])
                                complete=beat_aligned_time(complete+visual[Setting::PulseOffset],beats.onsets())-visual[Setting::PulseOffset];
                            if(composition.directed_extension) complete+=reveal;
                            item.transform[3]=static_cast<float>(rotation_angle(cone_rates,complete,time));
                            item.transform[11]=static_cast<float>(piece<spectral_cone_rib_count?piece_progress:2);
                        } else if(!composition.directed_extension) {
                            // Aglomerado volumétrico: espessura estática equivalente à largura, não uma folha.
                            const float x=std::hypot(item.transform[0],item.transform[2]),z=std::hypot(item.transform[8],item.transform[10]);
                            if(z>0) { item.transform[8]*=x/z; item.transform[10]*=x/z; }
                        }
                        visible.push_back(item);
                    }
                    surface_batch(view,family==9?luminous_cone[piece]:crystals[variant][piece],std::move(visible),state,time,camera,reflection,weight,kind);
                }
            } else {
                std::vector<Instance> visible;
                for(std::size_t index=first;index<end;++index) {
                    const auto item=prepared(instances[index]);
                    const auto opacity=opacity_for(instances[index],item);
                    if(opacity>0) visible.push_back(surface_fade_instance(item,opacity));
                }
                const Mesh& mesh=family==4||family==7?sphere:family==5?torus:family==6?ribbon:family==3?panel:
                    family==12?diamond:family==13?plate:box;
                surface_batch(view,mesh,std::move(visible),state,time,camera,reflection,weight,kind);
            }
            first=end;
        }
    }
    void full_screen(std::uint16_t view,bgfx::ProgramHandle p,bgfx::TextureHandle texture) {
        bgfx::setVertexBuffer(0,quad.vertices); bgfx::setIndexBuffer(quad.indices);
        bgfx::setTexture(0,uniforms[5],texture);
        bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A);
        bgfx::submit(view,p);
    }
    void draw(const Parameters& state,double time,double camera) {
        stereo_timer.reset();
        if(!targets.width||!targets.height) { bgfx::frame(); return; }
        float view[16],projection[16];
        const float tunnel=composition.weights[2];
        const float height=static_cast<float>(visual[Setting::EyeHeight]*100),back=static_cast<float>(visual[Setting::EyeBack]*100);
        const float target_y=composition.directed_extension?height:height+(180-tunnel*180)*(1-composition.opening_weight);
        bx::mtxLookAt(view,bx::Vec3{0,height,-back},bx::Vec3{0,target_y,3000});
        bx::mtxProj(projection,static_cast<float>(visual[Setting::DesktopFov])+tunnel*12,static_cast<float>(targets.width)/targets.height,
            static_cast<float>(visual[Setting::NearPlane]*100),static_cast<float>(visual[Setting::FarPlane]*100),bgfx::getCaps()->homogeneousDepth);
        const float eye[]={0,height,-back,0}; bgfx::setUniform(uniforms[9],eye);
        eye_origin={0,height,-back};
        draw_scene(state,time,camera,view,projection,false,false,BGFX_INVALID_HANDLE);
    }
    void set_hand_tracking(const interaction::ObservationFrame& frame) {
        // Substituir sempre o frame completo; nenhuma pose é retida após perda/recentragem.
        for(std::size_t i=0;i<hand_geometry.size();++i)
            hand_geometry[i]=vr_mode&&frame.frame_status=="located"&&frame.xr_time_ns>0?
                make_hand_geometry(frame.hands[i],static_cast<float>(visual[Setting::VrEyeHeight]*100),
                    static_cast<float>(visual[Setting::VrEyeBack]*100)):HandGeometry{};
    }
    std::vector<interaction::SnapEvent> update_hand_snap(const interaction::ObservationFrame& frame) {
        auto events=snap_detector.update(frame);
        if(!vr_mode||frame.frame_status!="located"||frame.xr_time_ns<=0||!frame.head.position_valid||
           !frame.head.position_tracked||!frame.head.orientation_valid||!frame.head.orientation_tracked) {
            snap_bursts={}; snap_geometry={}; snap_time.reset(); return {};
        }
        if(snap_time&&(frame.origin_epoch!=snap_epoch||frame.xr_time_ns<=*snap_time||frame.xr_time_ns-*snap_time>80'000'000))
            snap_bursts={};
        snap_time=frame.xr_time_ns; snap_epoch=frame.origin_epoch;
        for(const auto& event:events) snap_bursts[event.hand]=event;
        for(std::size_t i=0;i<2;++i) {
            snap_geometry[i]={}; snap_fade[i]=0;
            if(!interaction::snap_hand_usable(frame.hands[i])) snap_bursts[i].reset();
            if(!snap_bursts[i]) continue;
            snap_fade[i]=snap_particle_fade(*snap_bursts[i],frame.xr_time_ns);
            if(snap_fade[i]<=0) { snap_bursts[i].reset(); continue; }
            snap_geometry[i]=make_snap_geometry(*snap_bursts[i],frame.xr_time_ns,
                static_cast<float>(visual[Setting::VrEyeHeight]*100),static_cast<float>(visual[Setting::VrEyeBack]*100));
        }
        return events;
    }
    std::vector<interaction::PencilEvent> update_light_pencil(const interaction::ObservationFrame& frame) {
        auto events=light_pencil.update(frame);
        for(std::size_t hand=0;hand<2;++hand)
            pencil_geometry[hand]=vr_mode?make_pencil_geometry(light_pencil.points(hand),frame.xr_time_ns,
                static_cast<float>(visual[Setting::VrEyeHeight]*100),static_cast<float>(visual[Setting::VrEyeBack]*100)):PencilGeometry{};
        return events;
    }
    void draw_hands() {
        bgfx::VertexLayout layout;
        layout.begin().add(bgfx::Attrib::Position,3,bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,3,bgfx::AttribType::Float).end();
        static_assert(sizeof(HandVertex)==6*sizeof(float));
        constexpr float colours[2][4]={{0.10f,0.65f,0.85f,1},{0.85f,0.40f,0.12f,1}};
        for(std::size_t i=0;i<4;++i) {
            const bool spark=i>=2;
            const auto hand=i%2;
            const auto& mesh=spark?snap_geometry[hand]:hand_geometry[hand];
            if(mesh.indices.empty()) continue;
            const auto vertices=static_cast<std::uint32_t>(mesh.vertices.size()),indices=static_cast<std::uint32_t>(mesh.indices.size());
            bgfx::TransientVertexBuffer vb; bgfx::TransientIndexBuffer ib;
            if(!bgfx::allocTransientBuffers(&vb,layout,vertices,&ib,indices)) continue;
            std::memcpy(vb.data,mesh.vertices.data(),vertices*sizeof(HandVertex));
            std::memcpy(ib.data,mesh.indices.data(),indices*sizeof(std::uint16_t));
            bgfx::setVertexBuffer(0,&vb); bgfx::setIndexBuffer(&ib);
            const float glow[]{2.2f*snap_fade[hand],1.7f*snap_fade[hand],0.8f*snap_fade[hand],1};
            bgfx::setUniform(hand_colour,spark?glow:colours[hand]);
            // Profundidade partilhada com a cena; sem reflexo, fog ou avanço virtual.
            bgfx::setState(spark?(BGFX_STATE_WRITE_RGB|BGFX_STATE_DEPTH_TEST_LESS|BGFX_STATE_BLEND_ADD):
                (BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_WRITE_Z|BGFX_STATE_DEPTH_TEST_LESS));
            bgfx::submit(1,programs[7]);
        }
        bgfx::VertexLayout pencil_layout;
        pencil_layout.begin().add(bgfx::Attrib::Position,3,bgfx::AttribType::Float)
            .add(bgfx::Attrib::Normal,3,bgfx::AttribType::Float).add(bgfx::Attrib::TexCoord0,2,bgfx::AttribType::Float).end();
        static_assert(sizeof(PencilVertex)==8*sizeof(float));
        for(const auto& mesh:pencil_geometry) {
            if(mesh.indices.empty()) continue;
            bgfx::TransientVertexBuffer vb; bgfx::TransientIndexBuffer ib;
            if(!bgfx::allocTransientBuffers(&vb,pencil_layout,static_cast<std::uint32_t>(mesh.vertices.size()),
                                           &ib,static_cast<std::uint32_t>(mesh.indices.size()))) continue;
            std::memcpy(vb.data,mesh.vertices.data(),mesh.vertices.size()*sizeof(PencilVertex));
            std::memcpy(ib.data,mesh.indices.data(),mesh.indices.size()*sizeof(std::uint16_t));
            bgfx::setVertexBuffer(0,&vb); bgfx::setIndexBuffer(&ib);
            constexpr float colour[]{0.65f,1.30f,1.65f,1};
            bgfx::setUniform(hand_colour,colour);
            bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_DEPTH_TEST_LESS|BGFX_STATE_BLEND_ADD);
            bgfx::submit(1,programs[8]);
        }
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
        auto matrices=eye.matrices;
        const float dy=static_cast<float>(visual[Setting::VrEyeHeight]*100-160),dz=static_cast<float>(450-visual[Setting::VrEyeBack]*100);
        for(int i=0;i<3;++i) matrices.view[12+i]-=matrices.view[4+i]*dy+matrices.view[8+i]*dz;
        matrices.eye_world.y+=dy; matrices.eye_world.z+=dz;
        auto head_world=eye.head_world;
        head_world[13]+=dy; head_world[14]+=dz;
        std::array<float,16> head_to_eye{};
        for(int col=0;col<4;++col) for(int row=0;row<4;++row) for(int k=0;k<4;++k)
            head_to_eye[col*4+row]+=matrices.view[k*4+row]*head_world[col*4+k];
        stereo_timer=time_display_uniforms_vr(time_display,matrices.projection,head_to_eye);
        const float projection_near=static_cast<float>(visual[Setting::NearPlane]*100),projection_far=static_cast<float>(visual[Setting::FarPlane]*100);
        matrices.projection[10]=projection_far/(projection_far-projection_near); matrices.projection[14]=-projection_far*projection_near/(projection_far-projection_near);
        const auto& p=matrices.eye_world;
        eye_origin={p.x,p.y,p.z};
        const float origin[]={p.x,p.y,p.z,0}; bgfx::setUniform(uniforms[9],origin);
        draw_scene(state,time,camera,matrices.view.data(),matrices.projection.data(),eye.srgb,mirror,found->second);
    }
    void draw_scene(const Parameters& state,double time,double camera,const float* view,const float* projection,
        bool srgb,bool mirror,bgfx::FrameBufferHandle output) {
        if(composition.directed_extension&&time<last_time) {
            cone_rates={{0,0,visual[Setting::ConeRotation]*visual[Setting::AnimationSpeed]*3.141592653589793/180}};
            ring_alignment_camera.reset();
        }
        if(time<visual[Setting::RingAlignStart]) ring_alignment_camera.reset();
        else if(composition.directed_extension&&!ring_alignment_camera) {
            ring_alignment_speed=state.motion_speed/0.3;
            ring_formation_speed=visual[Setting::CameraSpeed]*std::max(1.0/0.3,ring_alignment_speed);
            ring_alignment_lead=required_ring_lead(ring_formation_speed,visual);
            ring_alignment_camera=camera-directed_camera_step(visual[Setting::RingAlignStart],time,state.motion_speed,visual);
        }
        last_time=time;
        transparent_draws.clear();
        std::copy(view,view+16,eye_view.begin());
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
        static_assert(sizeof(Instance)==80);
        if(composition.opening_weight>0) draw_opening(state,time,camera);
        for(std::size_t i=0;i<scenes.size();++i) {
            const float weight=composition.weights[i];
            if(weight<=0) continue;
            const auto& items=legacy_field?legacy_instances:(composition.directed_extension?(i<2?extension_scenes[i]:directed_tunnel):scenes[i]);
            auto layer_state=state;
            layer_state.geometry_density*=composition.geometry_density_multiplier;
            if(i<2) boxes(0,items,layer_state,time,camera,true,weight,static_cast<float>(i));
            boxes(1,items,layer_state,time,camera,false,weight,static_cast<float>(i));
        }
        if(composition.weights[2]<1) {
        scene_uniforms(state,time,camera,false,1-composition.weights[2],composition.weights[1]);
        Instance floor;
        floor.transform={50000,0,0,0, 0,1,0,0, 0,0,50000,0, 0,0,0,1};
        floor.variation=2.0f;
        bgfx::InstanceDataBuffer plane;
        bgfx::allocInstanceDataBuffer(&plane,1,sizeof(Instance));
        std::memcpy(plane.data,&floor,sizeof(floor));
        bgfx::setVertexBuffer(0,ground.vertices); bgfx::setIndexBuffer(ground.indices);
        bgfx::setInstanceDataBuffer(&plane);
        bgfx::setTexture(0,uniforms[4],bgfx::getTexture(targets.reflection));
        const auto floor_state=BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_DEPTH_TEST_LESS;
        bgfx::setState(floor_state|(composition.weights[2]>0?BGFX_STATE_BLEND_ALPHA:BGFX_STATE_WRITE_Z));
        bgfx::submit(1,programs[1]);
        }
        if(vr_mode) draw_hands();
        flush_transparency(state,time,camera);
        full_screen(2,programs[2],bgfx::getTexture(targets.scene));
        const float horizontal[]={1.0f/targets.half_width,0,0,0};
        bgfx::setUniform(uniforms[7],horizontal);
        full_screen(3,programs[3],bgfx::getTexture(targets.blur[0]));
        const float vertical[]={0,1.0f/targets.half_height,0,0};
        bgfx::setUniform(uniforms[7],vertical);
        full_screen(4,programs[3],bgfx::getTexture(targets.blur[1]));
        scene_uniforms(state,time,camera,false);
        bgfx::setUniform(uniforms[10],sky);
        const float post[]={state.bloom,srgb?1.0f:0.0f,composition.weights[2],composition.weights[1]}; bgfx::setUniform(uniforms[8],post);
        bgfx::setTexture(1,uniforms[6],bgfx::getTexture(targets.blur[2]));
        const auto timer=stereo_timer.value_or(time_display_uniforms(time_display,targets.width,targets.height));
        bgfx::setUniform(timer_uniform,timer.data(),5);
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
    void draw_opening(const Parameters& state,double time,double camera) {
        for(const auto& group:opening.groups) {
            if(!directed_group_visible(group,time,visual)) continue;
            const auto start=directed_group_start(group,visual);
            const float presence=opening_art_presence(group.role,time,start,visual);
            if(presence<=0) continue;
            float fade=1;
            if(group.stage!=OpeningStage::Forest) fade=std::clamp(static_cast<float>((visual[group.stage==OpeningStage::Dome?Setting::DomeEnd:Setting::IntroEnd]-time)/visual[Setting::StageFade]),0.0f,1.0f);
            std::vector<Instance> visible;
            visible.reserve(group.instances.size());
            const float cull=static_cast<float>(std::max(group.cull_distance*visual[Setting::CullMultiplier],
                (visual[Setting::RetainBehind]+visual[Setting::BehindFade])*100));
            for(const auto& original:group.instances) {
                const float density=static_cast<float>(visual.family_density(group.role));
                if(original.padding>=(group.role==ArtRole::Pyramid?density:presence*density)) continue;
                auto instance=directed_instance_with_motion(original,group,motion(time),visual);
                const auto& t=instance.transform;
                const float dz=t[14]-static_cast<float>(camera)-eye_origin[2];
                float radius=opening_radii.at(group.mesh)*std::sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]+t[4]*t[4]+t[5]*t[5]+t[6]*t[6]+t[8]*t[8]+t[9]*t[9]+t[10]*t[10]);
                if(group.floor_anchored) radius+=static_cast<float>(std::max(visual[Setting::PoleAmplitude],visual[Setting::MiniPoleMinimum])*100);
                if(std::hypot(t[12]-eye_origin[0],t[13]-eye_origin[1],dz)<=cull+radius) {
                    const auto value=(original.padding*1024.0f+0.5f)*0.61803398875f;
                    const auto rank=value-std::floor(value);
                    const auto layout=std::clamp((composition.geometry_density_multiplier-rank)*25+1,0.0f,1.0f)
                        *std::min(1.0f,composition.geometry_density_multiplier*25)
                        *previous_piece_visibility(rank,composition.previous_exit_progress)
                        *rear_piece_visibility(dz,radius,visual);
                    if(layout<=0) continue;
                    instance=surface_fade_instance(instance,layout);
                    const auto weight=composition.opening_weight*fade*presence;
                    if(weight*layout<0.99999) {
                        for(std::uint16_t pass=0;pass<2;++pass)
                            transparent_draws.push_back({pass,&opening_meshes.at(group.mesh),&group,instance,weight,0,presence,draw_depth(instance,camera,pass==0,false)});
                    } else visible.push_back(instance);
                }
            }
            const auto& mesh=opening_meshes.at(group.mesh);
            for(std::size_t offset=0;offset<visible.size();offset+=2048) {
                const auto n=static_cast<std::uint32_t>(std::min<std::size_t>(2048,visible.size()-offset));
                if(bgfx::getAvailInstanceDataBuffer(n,sizeof(Instance))<n) throw std::runtime_error("Opening instance buffer exhausted");
                bgfx::InstanceDataBuffer data; bgfx::allocInstanceDataBuffer(&data,n,sizeof(Instance));
                std::memcpy(data.data,visible.data()+offset,n*sizeof(Instance));
                for(std::uint16_t pass=0;pass<2;++pass) {
                    scene_uniforms(state,time,camera,pass==0,composition.opening_weight*fade*presence,0);
                    const float art[]={beats.sample(time+visual[Setting::PulseOffset]),presence,static_cast<float>(time),static_cast<float>(group.role)};
                    bgfx::setUniform(uniforms[14],art);
                    auto tuning=visual_uniforms(visual,motion(time),time,composition.directed_extension);
                    const auto colour=visual.family_colour(group.role);
                    tuning[20]={colour.r,colour.g,colour.b,static_cast<float>(visual.family_emission(group.role))};
                    bgfx::setUniform(uniforms[15],tuning.data(),22);
                    bgfx::setVertexBuffer(0,mesh.vertices); bgfx::setIndexBuffer(mesh.indices); bgfx::setInstanceDataBuffer(&data);
                    const auto index=group.material==1?1:group.material==2?2:0;
                    bgfx::setTexture(0,uniforms[13],opening_details[index]);
                    bgfx::setState(BGFX_STATE_WRITE_RGB|BGFX_STATE_WRITE_A|BGFX_STATE_DEPTH_TEST_LESS|BGFX_STATE_WRITE_Z);
                    bgfx::submit(pass,programs[6]);
                }
            }
        }
    }
};
Renderer::Renderer(void* window,std::uint16_t w,std::uint16_t h,const std::filesystem::path& shader_dir,void* graphics_device,
    std::function<void()> before_shutdown,const std::filesystem::path& soundtrack,const VisualConfig& config)
    : impl_(std::make_unique<Impl>(window,w,h,shader_dir,graphics_device,std::move(before_shutdown),soundtrack,config)) {}
Renderer::~Renderer()=default;
void Renderer::resize(std::uint16_t w,std::uint16_t h) { impl_->resize(w,h); }
void Renderer::draw(const Parameters& state,double time,double camera) { impl_->draw(state,time,camera); }
void Renderer::draw_vr_eye(const Parameters& state,double time,double camera,const vr::EyeView& eye,bool mirror) {
    impl_->draw_vr_eye(state,time,camera,eye,mirror);
}
void Renderer::flush() { bgfx::frame(); }
void Renderer::set_scene_layout(SceneLayout layout) {
    impl_->legacy_field=layout==SceneLayout::Field;
    impl_->composition=static_composition(layout);
}
void Renderer::set_composition(const Composition& composition) {
    impl_->legacy_field=false;
    impl_->composition=composition;
}
void Renderer::set_time_display(const TimeDisplay& display) { impl_->time_display=display; }
void Renderer::set_hand_tracking(const interaction::ObservationFrame& frame) { impl_->set_hand_tracking(frame); }
std::vector<interaction::SnapEvent> Renderer::update_hand_snap(const interaction::ObservationFrame& frame) { return impl_->update_hand_snap(frame); }
std::vector<interaction::PencilEvent> Renderer::update_light_pencil(const interaction::ObservationFrame& frame) { return impl_->update_light_pencil(frame); }
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
std::size_t Renderer::opening_mesh_count() const { return impl_->opening_meshes.size(); }
std::size_t Renderer::beat_count() const { return impl_->beats.beat_count(); }
float Renderer::beat_strength(double time) const { return impl_->beats.sample(time+impl_->visual[Setting::PulseOffset]); }
int Renderer::ring_pulse_group(double time) const { return impl_->beats.pulse_group(time+impl_->visual[Setting::PulseOffset],impl_->visual[Setting::RingPulseStart]+impl_->visual[Setting::PulseOffset]); }
double Renderer::soundtrack_duration() const { return impl_->beats.duration_seconds(); }
std::size_t Renderer::route_adjustments() const { return impl_->adjusted_instances; }
void Renderer::set_visual_config(const VisualConfig& config,std::optional<double> time) { impl_->set_config(config,time.value_or(impl_->last_time)); }
}
