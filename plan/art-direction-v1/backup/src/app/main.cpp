#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "control/protocol.hpp"
#include "control/scene_controller.hpp"
#include "net/control_server.hpp"
#include "render/renderer.hpp"
#include "audio/player.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {
using namespace fantasynt;
using Clock=std::chrono::steady_clock;
using Json=nlohmann::json;
std::string hex_bytes(std::string_view bytes) {
    constexpr char digits[]="0123456789abcdef";
    std::string result;
    result.reserve(bytes.size()*2);
    for(unsigned char byte:bytes) {
        result.push_back(digits[byte>>4]);
        result.push_back(digits[byte&15]);
    }
    return result;
}
struct Options {
    int width{1280},height{720};
    std::uint16_t port{47631};
    std::uint64_t frames{};
    bool hidden{};
    bool vr{},xr_probe{};
    bool sequence{true},opening_only{true},play{},paused{},silent{};
    double start_time{};
    std::optional<double> fixed_time;
    float vr_scale{1.0f};
    std::filesystem::path capture,shaders,audio;
    std::string preset;
    SceneLayout layout{SceneLayout::Opening};
};
std::uint64_t integer(std::string_view text) {
    std::uint64_t value{};
    const auto [last,error]=std::from_chars(text.data(),text.data()+text.size(),value);
    if(error!=std::errc{}||last!=text.data()+text.size()) throw std::runtime_error("Invalid integer argument");
    return value;
}
Options options(int argc,char** argv) {
    Options result;
    for(int i=1;i<argc;++i) {
        const std::string_view arg=argv[i];
        if(arg=="--hidden") { result.hidden=true; continue; }
        if(arg=="--silent") { result.silent=true; continue; }
        if(arg=="--vr") { result.vr=true; continue; }
        if(arg=="--xr-probe") { result.xr_probe=true; continue; }
        if(arg=="--help") {
            std::cout<<"Fantasynt --scene opening|sequence|night|landscape|tunnel|field --start-time SECONDS --fixed-time SECONDS --play --paused --silent --audio WAV --vr --vr-scale 0.4..1.0 (default 1.0) --xr-probe --port N --frames N --hidden --capture FILE --width N --height N --preset calm|balanced|energetic --shaders DIRECTORY\n";
            std::exit(0);
        }
        if(arg=="--play") { result.play=true; result.paused=false; continue; }
        if(arg=="--paused") { result.paused=true; result.play=false; continue; }
        if(i+1>=argc) throw std::runtime_error("Missing argument value");
        const std::string value=argv[++i];
        if(arg=="--port") {
            auto number=integer(value);
            if(!number||number>65535) throw std::runtime_error("Invalid port");
            result.port=static_cast<std::uint16_t>(number);
        } else if(arg=="--frames") {
            result.frames=integer(value);
            if(!result.frames||result.frames>100000000) throw std::runtime_error("Invalid frame count");
        } else if(arg=="--width"||arg=="--height") {
            auto number=integer(value);
            if(number<64||number>8192) throw std::runtime_error("Window dimensions must be 64..8192");
            if(arg=="--width") result.width=static_cast<int>(number);
            else result.height=static_cast<int>(number);
        } else if(arg=="--vr-scale") {
            const auto [end,error]=std::from_chars(value.data(),value.data()+value.size(),result.vr_scale);
            if(error!=std::errc{}||end!=value.data()+value.size()||!std::isfinite(result.vr_scale)||result.vr_scale<0.4f||result.vr_scale>1.0f)
                throw std::runtime_error("VR scale must be 0.4..1.0");
        } else if(arg=="--scene") {
            result.opening_only=value=="opening";
            result.sequence=value=="sequence"||result.opening_only;
            if(result.opening_only) result.layout=SceneLayout::Opening;
            else if(value=="night") result.layout=SceneLayout::Night;
            else if(value=="landscape") result.layout=SceneLayout::Landscape;
            else if(value=="tunnel") result.layout=SceneLayout::Tunnel;
            else if(value=="field") result.layout=SceneLayout::Field;
            else if(!result.sequence) throw std::runtime_error("Unknown scene");
        } else if(arg=="--start-time"||arg=="--fixed-time") {
            double seconds{};
            const auto [end,error]=std::from_chars(value.data(),value.data()+value.size(),seconds);
            if(error!=std::errc{}||end!=value.data()+value.size()||!std::isfinite(seconds)||seconds<0||seconds>sequence_duration)
                throw std::runtime_error("Time must be 0..424 finite seconds");
            if(arg=="--start-time") result.start_time=seconds;
            else result.fixed_time=seconds;
        } else if(arg=="--capture") result.capture=std::filesystem::u8path(value);
        else if(arg=="--shaders") result.shaders=std::filesystem::u8path(value);
        else if(arg=="--audio") result.audio=std::filesystem::u8path(value);
        else if(arg=="--preset") {
            if(value!="calm"&&value!="balanced"&&value!="energetic") throw std::runtime_error("Unknown preset");
            result.preset=value;
        } else throw std::runtime_error("Unknown argument: "+std::string(arg));
    }
    if(result.opening_only&&(result.start_time>opening_duration||result.fixed_time.value_or(0)>opening_duration))
        throw std::runtime_error("Opening time must be 0..120; use --scene sequence for later times");
    return result;
}
struct SDLSession {
    SDLSession() { if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) throw std::runtime_error(SDL_GetError()); }
    ~SDLSession() { SDL_Quit(); }
};
void apply_preset(SceneController& scene,const std::filesystem::path& directory,const std::string& name) {
    std::ifstream input(directory/(name+".json"));
    if(!input) throw std::runtime_error("Preset file missing: "+name);
    std::string payload((std::istreambuf_iterator<char>(input)),{});
    auto result=parse_command(payload);
    if(auto command=std::get_if<Command>(&result)) scene.submit(*command);
    else throw std::runtime_error("Invalid preset: "+name);
}
int run(const Options& args) {
    if(args.xr_probe) { std::cout<<vr::probe_openxr()<<std::endl; return 0; }
    SDLSession session;
    const auto flags=SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY|(args.hidden?SDL_WINDOW_HIDDEN:0);
    std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Fantasynt 0.5.0 | O opening Q restart N night L landscape T tunnel F field | Space pause | 1 2 3 presets",args.width,args.height,flags),SDL_DestroyWindow);
    if(!window) throw std::runtime_error(SDL_GetError());
    void* native=SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr);
    if(!native) throw std::runtime_error("Native window handle unavailable");
    const std::filesystem::path base=std::filesystem::u8path(SDL_GetBasePath());
    ControlServer server(args.port);
    std::unique_ptr<vr::OpenXRSession> xr;
    if(args.vr) xr=std::make_unique<vr::OpenXRSession>(args.vr_scale);
    Renderer renderer(native,static_cast<std::uint16_t>(args.width),static_cast<std::uint16_t>(args.height),
        args.shaders.empty()?base/"shaders":args.shaders,xr?xr->graphics_device():nullptr,
        args.vr?std::function<void()>([&xr] { xr.reset(); }):std::function<void()>{});
    renderer.set_scene_layout(args.layout);
    bool sequence=args.sequence;
    bool opening_only=args.opening_only;
    SceneLayout active_layout=args.layout;
    double sequence_seconds=args.fixed_time.value_or(args.start_time);
    std::unique_ptr<AudioPlayer> audio;
    auto ensure_audio=[&] {
        const double duration=opening_only?opening_duration:sequence_duration;
        if(audio&&audio->duration()!=duration) audio.reset();
        if(!audio&&!args.silent&&!args.fixed_time) {
            audio=std::make_unique<AudioPlayer>(args.audio.empty()?base/"audio/ChezNous_MASTER.wav":args.audio,duration);
            audio->seek(sequence_seconds);
        }
    };
    if(sequence) ensure_audio();
    auto composition_at=[&](double t) { return opening_only?sample_opening(t):sample_sequence(t); };
    if(sequence) renderer.set_composition(composition_at(sequence_seconds));
    SceneController scene;
    if(!args.preset.empty()) apply_preset(scene,base/"examples",args.preset);
    std::ofstream events("fantasynt-events.jsonl",std::ios::app);
    if(!events) throw std::runtime_error("Cannot open event log");
    std::cout<<Json{{"type","ready"},{"port",server.port()},{"device",renderer.device_description()},
        {"mode",xr?"openxr":"desktop"},{"runtime",xr?xr->description():""},{"sequence",sequence},
        {"audio_enabled",audio!=nullptr},{"audio_driver",audio?SDL_GetCurrentAudioDriver():""},
        {"opening_meshes",renderer.opening_mesh_count()},{"vr_scale",args.vr_scale}}.dump()<<std::endl;
    const auto start=Clock::now();
    auto previous=start;
    std::uint64_t frame_count{};
    double camera=sequence?sample_camera_position(sequence_seconds):0,latency_total{},latency_max{};
    std::uint64_t commands{};
    bool quit=false,paused=args.paused||(args.vr&&!args.play),minimized=false,capture_requested=false;
    auto select_layout=[&](SceneLayout layout) {
        sequence=false; active_layout=layout;
        if(audio) audio->set_paused(true);
        renderer.set_scene_layout(layout); camera=0;
    };
    while(!quit && (!args.frames||frame_count<args.frames)) {
        const auto tick=Clock::now();
        const double dt=std::chrono::duration<double>(tick-previous).count(); previous=tick;
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_EVENT_QUIT) quit=true;
            else if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                switch(event.key.key) {
                case SDLK_ESCAPE: quit=true; break;
                case SDLK_SPACE: paused=!paused; break;
                case SDLK_1: apply_preset(scene,base/"examples","calm"); break;
                case SDLK_2: apply_preset(scene,base/"examples","balanced"); break;
                case SDLK_3: apply_preset(scene,base/"examples","energetic"); break;
                case SDLK_R: apply_preset(scene,base/"examples","balanced"); break;
                case SDLK_Q:
                    sequence=true; sequence_seconds=0; camera=sample_camera_position(0);
                    ensure_audio(); if(audio) audio->seek(0); break;
                case SDLK_O:
                    opening_only=true; sequence=true; sequence_seconds=0; camera=sample_camera_position(0);
                    ensure_audio(); if(audio) audio->seek(0); break;
                case SDLK_N: select_layout(SceneLayout::Night); break;
                case SDLK_L: select_layout(SceneLayout::Landscape); break;
                case SDLK_T: select_layout(SceneLayout::Tunnel); break;
                case SDLK_F: select_layout(SceneLayout::Field); break;
                default: break;
                }
            } else if(event.type==SDL_EVENT_WINDOW_MINIMIZED) {
                minimized=true; renderer.resize(0,0);
            } else if(event.type==SDL_EVENT_WINDOW_RESTORED||event.type==SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                int width{},height{}; SDL_GetWindowSizeInPixels(window.get(),&width,&height);
                minimized=width<=0||height<=0;
                renderer.resize(static_cast<std::uint16_t>(std::clamp(width,0,8192)),
                    static_cast<std::uint16_t>(std::clamp(height,0,8192)));
            }
        }
        scene.advance(dt);
        for(const auto& message:server.drain()) {
            const double latency=std::chrono::duration<double,std::milli>(Clock::now()-message.received).count();
            auto result=parse_command(message.payload);
            Json receipt{{"schema_version",1},{"latency_ms",latency}};
            if(auto command=std::get_if<Command>(&result)) {
                scene.submit(*command);
                receipt["id"]=command->id; receipt["status"]="accepted";
                latency_total+=latency; latency_max=std::max(latency_max,latency); ++commands;
            } else {
                const auto& error=std::get<ValidationError>(result);
                if(!error.id.empty()) receipt["id"]=error.id;
                receipt["status"]="rejected"; receipt["error_code"]=error.code; receipt["fields"]=error.fields;
            }
            receipt["revision"]=scene.revision();
            receipt["current"]=parameters_to_json(scene.current());
            receipt["target"]=parameters_to_json(scene.target());
            server.reply(message.client_id,receipt.dump());
            events<<Json{{"received_seconds",std::chrono::duration<double>(Clock::now()-start).count()},
                {"payload_hex",hex_bytes(message.payload)},{"receipt",receipt}}.dump()<<'\n';
            events.flush();
            if(!events) throw std::runtime_error("Cannot write event log");
        }
        if(audio && sequence) {
            const double duration=opening_only?opening_duration:sequence_duration;
            audio->set_paused(paused||sequence_seconds>=duration);
            const double next=std::clamp(audio->position(),sequence_seconds,duration);
            camera+=sequence_camera_step(sequence_seconds,next,scene.current().motion_speed);
            sequence_seconds=next;
        } else if(!paused&&!args.fixed_time) {
            if(sequence) {
                const double next=std::min(opening_only?opening_duration:sequence_duration,sequence_seconds+std::min(dt,0.1));
                camera+=sequence_camera_step(sequence_seconds,next,scene.current().motion_speed);
                sequence_seconds=next;
            } else camera+=scene.current().motion_speed*250.0*std::min(dt,0.1);
        }
        if(sequence) renderer.set_composition(composition_at(sequence_seconds));
        else camera=std::fmod(camera,41600.0);
        std::optional<std::array<vr::EyeView,2>> eyes;
        if(xr) {
            if(!xr->poll_events()||quit) break;
            eyes=xr->begin_frame();
            if(!eyes) { SDL_Delay(10); continue; }
        }
        if(!args.capture.empty()&&!capture_requested&&!minimized &&
            ((args.frames&&frame_count+3>=args.frames)||(!args.frames&&frame_count>=120))) {
            renderer.request_capture(args.capture); capture_requested=true;
        }
        const double seconds=sequence?sequence_seconds:
            args.fixed_time.value_or(args.start_time+std::chrono::duration<double>(tick-start).count());
        if(eyes) {
            renderer.draw_vr_eye(scene.current(),seconds,camera,(*eyes)[0],true);
            renderer.draw_vr_eye(scene.current(),seconds,camera,(*eyes)[1],false);
            xr->end_frame();
        } else renderer.draw(scene.current(),seconds,camera);
        ++frame_count;
        const auto remaining=start+std::chrono::microseconds(16667*frame_count)-Clock::now();
        if(!xr&&remaining>Clock::duration::zero())
            SDL_DelayPrecise(static_cast<Uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(remaining).count()));
    }
    if(audio) {
        audio->set_paused(true);
        if(sequence) {
            const double next=std::clamp(audio->position(),sequence_seconds,opening_only?opening_duration:sequence_duration);
            camera+=sequence_camera_step(sequence_seconds,next,scene.current().motion_speed);
            sequence_seconds=next;
        }
    }
    if(!args.capture.empty()&&!capture_requested&&!minimized) { renderer.request_capture(args.capture); capture_requested=true; }
    if(capture_requested) {
        for(int i=0;i<60&&!renderer.capture_complete();++i) {
            renderer.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if(!renderer.capture_complete()) throw std::runtime_error("Screenshot did not complete");
    }
    const double seconds=std::chrono::duration<double>(Clock::now()-start).count();
    std::cout<<Json{{"type","performance"},{"frames",frame_count},{"seconds",seconds},{"sequence",sequence},
        {"sequence_seconds",sequence_seconds},{"composition_weights",sequence?composition_at(sequence_seconds).weights:static_composition(active_layout).weights},
        {"scene",sequence?(opening_only?"opening":"sequence"):"manual"},{"opening_active",sequence&&composition_at(sequence_seconds).opening_weight>0},
        {"camera_position",camera},{"atmosphere",sequence?std::array<float,3>{composition_at(sequence_seconds).atmosphere.r,
            composition_at(sequence_seconds).atmosphere.g,composition_at(sequence_seconds).atmosphere.b}:std::array<float,3>{}},
        {"audio_enabled",audio!=nullptr},{"audio_seconds",audio?audio->position():0},
        {"average_fps",frame_count/seconds},{"accepted_commands",commands},
        {"mean_delivery_ms",commands?latency_total/commands:0.0},{"max_delivery_ms",latency_max}}.dump()<<std::endl;
    return 0;
}
}
int main(int argc,char** argv) {
    try { return run(options(argc,argv)); }
    catch(const std::exception& e) { std::cerr<<"Error: "<<e.what()<<'\n'; return 1; }
}
