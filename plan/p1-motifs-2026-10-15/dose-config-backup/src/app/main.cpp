#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "control/protocol.hpp"
#include "control/scene_controller.hpp"
#include "net/control_server.hpp"
#include "render/renderer.hpp"
#include "audio/player.hpp"
#include "render/art_direction.hpp"
#include "control/session_recorder.hpp"
#include "interaction/observation.hpp"
#include "behavior/motif_executor.hpp"
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
    bool vr{},xr_probe{},scheduled_motifs{};
    bool vr_scale_set{},scene_set{},width_set{},height_set{};
    bool sequence{true},opening_only{true},play{},paused{},silent{};
    double start_time{};
    std::optional<double> fixed_time;
    float vr_scale{1.0f};
    std::filesystem::path capture,shaders,audio,config,record_session;
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
        if(arg=="--motifs-scheduled") { result.scheduled_motifs=true; continue; }
        if(arg=="--xr-probe") { result.xr_probe=true; continue; }
        if(arg=="--help") {
            std::cout<<"Fantasynt --config FILE.json --scene opening|sequence|night|landscape|tunnel|field --start-time SECONDS --fixed-time SECONDS --play --paused --silent --audio WAV --vr --vr-scale 0.4..1.0 --xr-probe --port N --frames N --hidden --capture FILE --record-session FILE.jsonl --motifs-scheduled --width N --height N --preset calm|balanced|energetic --shaders DIRECTORY | C timer | F5 reload config\n";
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
            if(arg=="--width") { result.width=static_cast<int>(number); result.width_set=true; }
            else { result.height=static_cast<int>(number); result.height_set=true; }
        } else if(arg=="--vr-scale") {
            result.vr_scale_set=true;
            const auto [end,error]=std::from_chars(value.data(),value.data()+value.size(),result.vr_scale);
            if(error!=std::errc{}||end!=value.data()+value.size()||!std::isfinite(result.vr_scale)||result.vr_scale<0.4f||result.vr_scale>1.0f)
                throw std::runtime_error("VR scale must be 0.4..1.0");
        } else if(arg=="--scene") {
            result.scene_set=true;
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
                throw std::runtime_error("Time must be within the soundtrack timeline");
            if(arg=="--start-time") result.start_time=seconds;
            else result.fixed_time=seconds;
        } else if(arg=="--capture") result.capture=std::filesystem::u8path(value);
        else if(arg=="--record-session") result.record_session=std::filesystem::u8path(value);
        else if(arg=="--shaders") result.shaders=std::filesystem::u8path(value);
        else if(arg=="--audio") result.audio=std::filesystem::u8path(value);
        else if(arg=="--config") result.config=std::filesystem::u8path(value);
        else if(arg=="--preset") {
            if(value!="calm"&&value!="balanced"&&value!="energetic") throw std::runtime_error("Unknown preset");
            result.preset=value;
        } else throw std::runtime_error("Unknown argument: "+std::string(arg));
    }
    return result;
}
struct SDLSession {
    SDLSession() { if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)) throw std::runtime_error(SDL_GetError()); }
    ~SDLSession() { SDL_Quit(); }
};
struct ScopeExit {
    std::function<void()> action;
    ~ScopeExit() { try { action(); } catch(...) {} }
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
    const std::filesystem::path base=std::filesystem::u8path(SDL_GetBasePath());
    const auto config_path=args.config.empty()?base/"visual-config.json":args.config;
    VisualConfigFile config_file(config_path);
    auto visual=load_visual_config(config_path);
    bool sequence=args.sequence,opening_only=args.opening_only;
    auto selected_layout=args.layout;
    if(!args.scene_set) {
        const auto selected=static_cast<int>(visual[Setting::InitialScene]);
        sequence=selected<2; opening_only=selected==0;
        const SceneLayout layouts[]={SceneLayout::Opening,SceneLayout::Opening,SceneLayout::Night,SceneLayout::Landscape,SceneLayout::Tunnel,SceneLayout::Field};
        selected_layout=layouts[selected];
    }
    if(opening_only&&(args.start_time>visual[Setting::OpeningDuration]||args.fixed_time.value_or(0)>visual[Setting::OpeningDuration]))
        throw std::runtime_error("Opening time exceeds configured duration; use --scene sequence for later times");
    if(args.scheduled_motifs&&(!sequence||!opening_only||!args.preset.empty()||
       std::abs(visual.parameters.light_intensity-0.6f)>1e-6||
       std::abs(visual[Setting::OpeningDuration]-sequence_duration)>1e-6))
        throw std::runtime_error("Scheduled P1 requires full Opening, base light 0.6 and no preset");
    const int width=args.width_set?args.width:static_cast<int>(visual[Setting::WindowWidth]);
    const int height=args.height_set?args.height:static_cast<int>(visual[Setting::WindowHeight]);
    const float vr_scale=args.vr_scale_set?args.vr_scale:static_cast<float>(visual[Setting::VrScale]);
    const auto startup_visual=visual;
    const bool startup_paused=args.paused||(!args.play&&!visual[Setting::AutoPlay]);
    const auto configured_audio=std::filesystem::u8path(visual.soundtrack);
    const auto audio_path=args.audio.empty()?(configured_audio.is_absolute()?configured_audio:base/configured_audio):args.audio;
    const auto flags=SDL_WINDOW_RESIZABLE|SDL_WINDOW_HIGH_PIXEL_DENSITY|(args.hidden?SDL_WINDOW_HIDDEN:0);
    std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Fantasynt 0.12.0 | C timer | F5 reload config | Space pause | Q restart",width,height,flags),SDL_DestroyWindow);
    if(!window) throw std::runtime_error(SDL_GetError());
    void* native=SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr);
    if(!native) throw std::runtime_error("Native window handle unavailable");
    ControlServer server(args.port);
    std::unique_ptr<vr::OpenXRSession> xr;
    if(args.vr) xr=std::make_unique<vr::OpenXRSession>(vr_scale);
    Renderer renderer(native,static_cast<std::uint16_t>(width),static_cast<std::uint16_t>(height),
        args.shaders.empty()?base/"shaders":args.shaders,xr?xr->graphics_device():nullptr,
        args.vr?std::function<void()>([&xr] { xr.reset(); }):std::function<void()>{},audio_path,visual);
    const double soundtrack_end=renderer.soundtrack_duration();
    const auto playback_end=[&](const VisualConfig& c) { return std::min(soundtrack_end,opening_only?c[Setting::OpeningDuration]:sequence_duration); };
    if(sequence&&(args.start_time>playback_end(visual)||args.fixed_time.value_or(0)>playback_end(visual)))
        throw std::runtime_error("Opening time exceeds soundtrack duration");
    renderer.set_scene_layout(selected_layout);
    SceneLayout active_layout=selected_layout;
    double sequence_seconds=args.fixed_time.value_or(args.start_time);
    std::unique_ptr<AudioPlayer> audio;
    auto ensure_audio=[&] {
        const double duration=playback_end(visual);
        if(audio&&audio->duration()!=duration) audio.reset();
        if(!audio&&!args.silent&&!args.fixed_time) {
            audio=std::make_unique<AudioPlayer>(audio_path,duration,args.scheduled_motifs);
            audio->seek(sequence_seconds);
        }
    };
    if(sequence) ensure_audio();
    auto composition_at=[&](double t) { return directed_composition(t,opening_only,visual,playback_end(visual)); };
    if(sequence) renderer.set_composition(composition_at(sequence_seconds));
    SceneController scene(visual.parameters);
    bool paused=startup_paused;
    std::unique_ptr<SessionRecorder> recording;
    std::string recording_id;
    auto recording_path=args.record_session;
    if(args.scheduled_motifs&&recording_path.empty()) recording_path=std::filesystem::path("logs")/("p1-"+new_session_id()+".jsonl");
    const auto recording_start_ns=interaction::monotonic_now_ns();
    behavior::MotifExecutor motifs(args.scheduled_motifs);
    bool technical_paused=false;
    std::optional<std::uint64_t> motif_origin_epoch;
    auto motif_manifest=[&] {
        Json plans=Json::array();
        for(const auto& p:behavior::agenda) plans.push_back({{"id",p.id},{"start",p.start},{"duration",p.duration},
            {"origin",p.origin},{"planned_ancestor",p.ancestor}});
        return Json{{"enabled",args.scheduled_motifs},{"version",behavior::repertoire_version},{"agenda",plans},
            {"curve","long_dip_smoothstep"},{"entry_seconds",2},{"exit_seconds",2},
            {"light_depth",behavior::light_depth},{"gain_depth",behavior::gain_depth},
            {"neutral_deadline",behavior::neutral_deadline},{"body_decisions",false},
            {"audio_gain_method","PCM16_per_sample_precomputed_before_SDL_queue"},
            {"configuration_frozen",args.scheduled_motifs},{"fixed_preview",args.fixed_time.has_value()}};
    };
    if(!recording_path.empty()) {
        recording_id=new_session_id();
        recording=std::make_unique<SessionRecorder>(recording_path,Json{
            {"session_id",recording_id},{"condition",args.scheduled_motifs?"p1_scheduled":xr?"hand_visualization":"observe_only"},{"app_version","0.12.0"},
            {"motif_executor",motif_manifest()},
            {"build_commit",FANTASYNT_BUILD_COMMIT},{"executable_sha256",file_sha256(base/"fantasynt.exe")},
            {"configuration_path",path_utf8(config_path)},{"configuration_sha256",file_sha256(config_path)},
            {"configuration",Json::parse(visual_config_json(visual))},
            {"soundtrack_path",path_utf8(audio_path)},{"soundtrack_sha256",file_sha256(audio_path)},
            {"playback_end_seconds",playback_end(visual)},{"mode",xr?"openxr":"desktop"},
            {"runtime",xr?xr->description():""},{"tracking",xr?xr->tracking_capabilities():Json(nullptr)},
            {"started_monotonic_ns",recording_start_ns},{"observation_publish_hz",20},
            {"audio_clock","SDL_consumed_frames_not_physical_output"},{"audio_driver",audio?SDL_GetCurrentAudioDriver():""},
            {"start_time_seconds",sequence_seconds},{"paused",paused},{"silent",args.silent},
            {"fixed_time",args.fixed_time?Json(*args.fixed_time):Json(nullptr)},
            {"acquisition_only",xr==nullptr&&!args.scheduled_motifs},{"hands_rendered",xr!=nullptr},
            {"hand_representation",xr?Json("joint_spheres_and_bones"):Json(nullptr)},
            {"hand_position_policy","valid_and_tracked"}});
    }
    // A visibilidade é controlada exclusivamente pela tecla C durante a sessão.
    bool timer_visible=false;
    auto record_motif_events=[&] {
        for(auto& event:motifs.drain_events()) {
            event["session_id"]=recording_id;
            event["received_monotonic_ns"]=interaction::monotonic_now_ns();
            if(recording) recording->submit(std::move(event));
        }
    };
    ScopeExit motif_cleanup{[&] {
        if(audio) audio->set_paused(true);
        motifs.stop("exception_or_scope_exit"); record_motif_events();
    }};
    // Só promover um plano depois de conhecer a disponibilidade XR do primeiro frame.
    motifs.restart(sequence_seconds,true,"session_start");
    record_motif_events();
    if(!args.preset.empty()) apply_preset(scene,base/"examples",args.preset);
    std::ofstream events("fantasynt-events.jsonl",std::ios::app);
    if(!events) throw std::runtime_error("Cannot open event log");
    const auto config_utf8=config_path.u8string();
    std::cout<<Json{{"type","ready"},{"version","0.12.0"},{"playback_end_seconds",playback_end(visual)},
        {"port",server.port()},{"device",renderer.device_description()},
        {"visual_config",std::string(reinterpret_cast<const char*>(config_utf8.data()),config_utf8.size())},
        {"mode",xr?"openxr":"desktop"},{"runtime",xr?xr->description():""},{"sequence",sequence},
        {"audio_enabled",audio!=nullptr},{"audio_driver",audio?SDL_GetCurrentAudioDriver():""},
        {"opening_meshes",renderer.opening_mesh_count()},{"vr_scale",vr_scale},{"paused",paused},
        {"tracking",xr?xr->tracking_capabilities():Json(nullptr)},
        {"hands_rendered",xr!=nullptr},{"hand_position_policy","valid_and_tracked"},
        {"motif_executor",motif_manifest()},
        {"recording_path",recording_path.empty()?Json(nullptr):Json(path_utf8(recording_path))},
        {"detected_bass_beats",renderer.beat_count()},{"route_adjustments",renderer.route_adjustments()}}.dump()<<std::endl;
    const auto start=Clock::now();
    auto previous=start;
    std::uint64_t frame_count{};
    double camera=sequence?directed_camera_position(sequence_seconds,visual,opening_only):0,latency_total{},latency_max{};
    std::uint64_t commands{};
    bool quit=false,minimized=false,capture_requested=false;
    std::uint64_t config_revision{};
    auto next_config_poll=start;
    interaction::ObservationHistory observation_history;
    std::int64_t next_observation_ns{};
    std::optional<interaction::ObservationFrame> published_observation;
    auto record_observation=[&](bool submitted,double time,const Parameters& render_state) {
        if(!recording) return;
        const auto now=interaction::monotonic_now_ns();
        const auto* observation=xr?&xr->observation():nullptr;
        const bool changed=observation&&(!published_observation||
            interaction::tracking_quality_changed(*published_observation,*observation));
        if(now<next_observation_ns&&!changed) return;
        next_observation_ns=now+50'000'000;
        const auto observed_motion=observation?observation_history.advance(*observation):interaction::MotionDescriptors{};
        const auto composition=sequence?composition_at(time):static_composition(active_layout);
        const auto effective=motifs.compose(render_state);
        const bool accepted=recording->submit({{"type","observation"},{"session_id",recording_id},
            {"received_monotonic_ns",now},{"session_elapsed_seconds",(now-recording_start_ns)/1e9},
            {"xr",observation?interaction::observation_to_json(*observation):Json(nullptr)},
            {"motion",observation?interaction::motion_to_json(observed_motion):Json(nullptr)},
            {"environment",{{"scene",sequence?(opening_only?"opening":"sequence"):"manual"},
                {"render_submitted",submitted},{"render_frame",frame_count},{"scene_seconds",time},
                {"audio_seconds",audio?Json(audio->position()):Json(nullptr)},
                {"audio_sampled_monotonic_ns",interaction::monotonic_now_ns()},
                {"paused",paused},{"session_paused",paused||technical_paused},{"technical_paused",technical_paused},
                {"audio_enabled",audio!=nullptr},{"master_gain",effective.gain},
                {"camera_virtual_z_cm",camera},{"base",parameters_to_json(scene.current())},
                {"directed",parameters_to_json(render_state)},
                {"effective",parameters_to_json(effective.effective)},
                {"semantic_modulation",args.scheduled_motifs?motifs.state():Json(nullptr)},
                {"light_contribution",effective.light_contribution},{"light_clamped",effective.clamped},
                {"config_revision",config_revision},{"controller_revision",scene.revision()},
                {"composition_weights",composition.weights},{"opening_weight",composition.opening_weight},
                {"exposure",composition.exposure},{"geometry_density_multiplier",composition.geometry_density_multiplier},
                {"fog_multiplier",composition.fog_multiplier},{"tunnel_light_weight",composition.tunnel_light_weight},
                {"previous_exit_progress",composition.previous_exit_progress},
                {"bass_pulse",renderer.beat_strength(time)},{"ring_pulse_group",renderer.ring_pulse_group(time)}}}});
        if(accepted&&observation) published_observation=*observation;
        if(!accepted) observation_history={};
    };
    auto select_layout=[&](SceneLayout layout) {
        sequence=false; active_layout=layout;
        if(audio) audio->set_paused(true);
        renderer.set_scene_layout(layout); camera=0;
    };
    while(!quit && (!args.frames||frame_count<args.frames)) {
        const auto tick=Clock::now();
        const double dt=std::chrono::duration<double>(tick-previous).count(); previous=tick;
        SDL_Event event;
        bool reload_config=false;
        while(SDL_PollEvent(&event)) {
            if(event.type==SDL_EVENT_QUIT) quit=true;
            else if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                if(args.scheduled_motifs&&event.key.key!=SDLK_ESCAPE&&event.key.key!=SDLK_SPACE&&
                   event.key.key!=SDLK_C&&event.key.key!=SDLK_Q) {
                    if(recording) recording->submit({{"type","control_rejected"},{"session_id",recording_id},
                        {"received_monotonic_ns",interaction::monotonic_now_ns()},
                        {"reason","p1_configuration_frozen"},{"key",event.key.key}});
                    continue;
                }
                switch(event.key.key) {
                case SDLK_ESCAPE: quit=true; break;
                case SDLK_SPACE: paused=!paused; break;
                case SDLK_F5: reload_config=true; break;
                case SDLK_C: timer_visible=!timer_visible; break;
                case SDLK_1: apply_preset(scene,base/"examples","calm"); break;
                case SDLK_2: apply_preset(scene,base/"examples","balanced"); break;
                case SDLK_3: apply_preset(scene,base/"examples","energetic"); break;
                case SDLK_R: apply_preset(scene,base/"examples","balanced"); break;
                case SDLK_Q:
                    sequence=true; sequence_seconds=0; camera=directed_camera_position(0,visual,opening_only);
                    ensure_audio(); if(audio) audio->seek(0);
                    motifs.restart(0,paused,"operator_restart"); record_motif_events(); break;
                case SDLK_O:
                    opening_only=true; sequence=true; sequence_seconds=0; camera=directed_camera_position(0,visual);
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
        if(!args.scheduled_motifs&&(reload_config||(visual[Setting::HotReload]&&tick>=next_config_poll))) {
            next_config_poll=tick+std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(visual[Setting::PollInterval]));
            bool held_audio=false;
            try {
                if(auto next=config_file.changed(reload_config)) {
                    const bool parameters_changed=parameters_to_json(next->parameters)!=parameters_to_json(visual.parameters);
                    if(audio) { audio->set_paused(true); held_audio=true; }
                    const double audio_before=audio?audio->position():sequence_seconds;
                    double effective_time=sequence?audio_before:args.fixed_time.value_or(std::chrono::duration<double>(tick-start).count()+args.start_time);
                    if(sequence&&opening_only) effective_time=std::min(effective_time,playback_end(*next));
                    std::unique_ptr<AudioPlayer> replacement;
                    if(audio&&opening_only&&next->operator[](Setting::OpeningDuration)!=visual[Setting::OpeningDuration]) {
                        replacement=std::make_unique<AudioPlayer>(audio_path,playback_end(*next));
                        replacement->seek(std::min(audio->position(),playback_end(*next)));
                    }
                    renderer.set_visual_config(*next,effective_time);
                    const bool restart_required=(!args.vr_scale_set&&next->operator[](Setting::VrScale)!=startup_visual[Setting::VrScale])||
                        (!args.width_set&&next->operator[](Setting::WindowWidth)!=startup_visual[Setting::WindowWidth])||
                        (!args.height_set&&next->operator[](Setting::WindowHeight)!=startup_visual[Setting::WindowHeight])||
                        (!args.scene_set&&next->operator[](Setting::InitialScene)!=startup_visual[Setting::InitialScene])||
                        (args.paused||(!args.play&&!next->operator[](Setting::AutoPlay)))!=startup_paused||
                        (args.audio.empty()&&next->soundtrack!=startup_visual.soundtrack);
                    visual=std::move(*next);
                    if(replacement) { audio=std::move(replacement); sequence_seconds=audio->position(); }
                    else if(sequence) sequence_seconds=effective_time;
                    if(parameters_changed) {
                        const auto command=parse_command(Json{{"schema_version",1},{"id","visual-config"},{"command","apply_parameters"},
                            {"transition_seconds",visual[Setting::ParameterTransition]},{"parameters",parameters_to_json(visual.parameters)}}.dump());
                        scene.submit(std::get<Command>(command));
                    }
                    ++config_revision;
                    if(recording) recording->submit({{"type","configuration_changed"},{"session_id",recording_id},
                        {"received_monotonic_ns",interaction::monotonic_now_ns()},
                        {"revision",config_revision},{"configuration",Json::parse(visual_config_json(visual))}});
                    std::cout<<Json{{"type","config_reloaded"},{"revision",config_revision},{"restart_required",restart_required},
                        {"audio_seconds_before",audio_before},{"audio_seconds_after",audio?audio->position():sequence_seconds}}.dump()<<std::endl;
                }
            } catch(const std::exception& error) {
                std::cerr<<Json{{"type","config_rejected"},{"error",error.what()}}.dump()<<std::endl;
            }
            if(held_audio&&audio) audio->set_paused(paused||!sequence||sequence_seconds>=playback_end(visual));
        }
        if(!args.scheduled_motifs||!paused) scene.advance(dt);
        for(const auto& message:server.drain()) {
            const double latency=std::chrono::duration<double,std::milli>(Clock::now()-message.received).count();
            auto result=parse_command(message.payload);
            Json receipt{{"schema_version",1},{"latency_ms",latency}};
            if(auto command=std::get_if<Command>(&result)) {
                receipt["id"]=command->id;
                if(args.scheduled_motifs) {
                    receipt["status"]="rejected"; receipt["error_code"]="p1_configuration_frozen";
                } else {
                    scene.submit(*command); receipt["status"]="accepted";
                    latency_total+=latency; latency_max=std::max(latency_max,latency); ++commands;
                }
            } else {
                const auto& error=std::get<ValidationError>(result);
                if(!error.id.empty()) receipt["id"]=error.id;
                receipt["status"]="rejected"; receipt["error_code"]=error.code; receipt["fields"]=error.fields;
            }
            receipt["revision"]=scene.revision();
            receipt["current"]=parameters_to_json(scene.current());
            receipt["target"]=parameters_to_json(scene.target());
            server.reply(message.client_id,receipt.dump());
            if(recording) recording->submit({{"type","command"},{"session_id",recording_id},
                {"received_monotonic_ns",interaction::monotonic_now_ns()},
                {"payload_hex",hex_bytes(message.payload)},{"receipt",receipt}});
            events<<Json{{"received_seconds",std::chrono::duration<double>(Clock::now()-start).count()},
                {"payload_hex",hex_bytes(message.payload)},{"receipt",receipt}}.dump()<<'\n';
            events.flush();
            if(!events) throw std::runtime_error("Cannot write event log");
        }
        std::optional<std::array<vr::EyeView,2>> eyes;
        std::string_view technical_reason="operator_pause";
        if(args.scheduled_motifs&&xr) {
            if(!xr->poll_events()||quit) break;
            eyes=xr->begin_frame();
            const auto& observation=xr->observation();
            const bool origin_changed=motif_origin_epoch&&*motif_origin_epoch!=observation.origin_epoch;
            motif_origin_epoch=observation.origin_epoch;
            const auto& h=observation.head;
            technical_paused=!eyes||!h.position_valid||!h.orientation_valid||
                !h.position_tracked||!h.orientation_tracked||origin_changed;
            if(technical_paused) technical_reason=origin_changed?"xr_origin_changed":!eyes?"xr_frame_unavailable":"xr_head_unusable";
        }
        const bool session_paused=paused||technical_paused||quit;
        if(audio && sequence) {
            const double duration=playback_end(visual);
            audio->set_paused(session_paused||sequence_seconds>=duration);
            const double next=std::clamp(audio->position(),sequence_seconds,duration);
            camera+=directed_camera_step(sequence_seconds,next,scene.current().motion_speed,visual,opening_only);
            sequence_seconds=next;
        } else if(!session_paused&&!args.fixed_time) {
            if(sequence) {
                const double next=std::min(playback_end(visual),sequence_seconds+std::min(dt,0.1));
                camera+=directed_camera_step(sequence_seconds,next,scene.current().motion_speed,visual,opening_only);
                sequence_seconds=next;
            } else camera+=scene.current().motion_speed/0.3*visual[Setting::CameraSpeed]*100*std::min(dt,0.1);
        }
        if(sequence) renderer.set_composition(composition_at(sequence_seconds));
        else camera=std::fmod(camera,41600.0);
        motifs.advance(sequence_seconds,session_paused,technical_reason);
        record_motif_events();
        if(xr&&!args.scheduled_motifs) {
            if(!xr->poll_events()||quit) break;
            eyes=xr->begin_frame();
        }
        if(xr&&!eyes) {
                const double time=sequence?sequence_seconds:args.fixed_time.value_or(
                    args.start_time+std::chrono::duration<double>(tick-start).count());
                const auto state=sequence&&opening_only?directed_visual_parameters(scene.current(),time,visual):scene.current();
                record_observation(false,time,state);
                SDL_Delay(10); continue;
        }
        if(!args.capture.empty()&&!capture_requested&&!minimized &&
            ((args.frames&&frame_count+3>=args.frames)||(!args.frames&&frame_count>=120))) {
            renderer.request_capture(args.capture); capture_requested=true;
        }
        const double seconds=sequence?sequence_seconds:
            args.fixed_time.value_or(args.start_time+std::chrono::duration<double>(tick-start).count());
        const auto render_state=sequence&&opening_only?directed_visual_parameters(scene.current(),seconds,visual):scene.current();
        const auto effective=motifs.compose(render_state);
        renderer.set_time_display({timer_visible,seconds,static_cast<float>(visual[Setting::TimerScale]),
            static_cast<float>(visual[Setting::TimerX]),static_cast<float>(visual[Setting::TimerY]),
            static_cast<float>(visual[Setting::TimerOpacity]),effective.envelope>0});
        if(eyes) {
            renderer.set_hand_tracking(xr->observation());
            renderer.draw_vr_eye(effective.effective,seconds,camera,(*eyes)[0],true);
            renderer.draw_vr_eye(effective.effective,seconds,camera,(*eyes)[1],false);
            xr->end_frame();
        } else renderer.draw(effective.effective,seconds,camera);
        record_observation(true,seconds,render_state);
        ++frame_count;
        const auto remaining=start+std::chrono::microseconds(16667*frame_count)-Clock::now();
        if(!xr&&remaining>Clock::duration::zero())
            SDL_DelayPrecise(static_cast<Uint64>(std::chrono::duration_cast<std::chrono::nanoseconds>(remaining).count()));
    }
    if(audio) {
        audio->set_paused(true);
        if(sequence) {
            const double next=std::clamp(audio->position(),sequence_seconds,playback_end(visual));
            camera+=directed_camera_step(sequence_seconds,next,scene.current().motion_speed,visual,opening_only);
            sequence_seconds=next;
        }
    }
    motifs.stop(quit?"operator_or_runtime_exit":"frame_limit_or_runtime_stop");
    record_motif_events();
    if(!args.capture.empty()&&!capture_requested&&!minimized) { renderer.request_capture(args.capture); capture_requested=true; }
    if(capture_requested) {
        for(int i=0;i<60&&!renderer.capture_complete();++i) {
            renderer.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        if(!renderer.capture_complete()) throw std::runtime_error("Screenshot did not complete");
    }
    const double seconds=std::chrono::duration<double>(Clock::now()-start).count();
    if(recording) {
        recording->submit({{"type","application_end"},{"session_id",recording_id},
            {"received_monotonic_ns",interaction::monotonic_now_ns()},{"rendered_frames",frame_count},
            {"scene_seconds",sequence_seconds},{"reason",quit?"operator_or_runtime_exit":"frame_limit"}});
        const auto stats=recording->close(true);
        std::cout<<Json{{"type","recording_result"},{"session_id",recording_id},{"path",path_utf8(recording_path)},
            {"accepted",stats.accepted},{"dropped",stats.dropped},{"write_failed",stats.write_failed},{"complete",stats.complete}}.dump()<<std::endl;
        if(stats.write_failed) throw std::runtime_error("Session recording write failed");
    }
    std::cout<<Json{{"type","performance"},{"frames",frame_count},{"seconds",seconds},{"sequence",sequence},
        {"sequence_seconds",sequence_seconds},{"composition_weights",sequence?composition_at(sequence_seconds).weights:static_composition(active_layout).weights},
        {"scene",sequence?(opening_only?"opening":"sequence"):"manual"},{"opening_active",sequence&&composition_at(sequence_seconds).opening_weight>0},
        {"camera_position",camera},{"atmosphere",sequence?std::array<float,3>{composition_at(sequence_seconds).atmosphere.r,
            composition_at(sequence_seconds).atmosphere.g,composition_at(sequence_seconds).atmosphere.b}:std::array<float,3>{}},
        {"audio_enabled",audio!=nullptr},{"audio_seconds",audio?audio->position():0},
        {"bass_pulse",renderer.beat_strength(sequence_seconds)},{"ring_pulse_group",renderer.ring_pulse_group(sequence_seconds)},
        {"version","0.12.0"},{"playback_end_seconds",playback_end(visual)},
        {"config_revision",config_revision},{"camera_speed_m_s",visual[Setting::CameraSpeed]},{"trace_speed_m_s",visual[Setting::TraceSpeed]},
        {"time_display_visible",timer_visible},{"time_display",format_time_display(sequence_seconds)},
        {"visual_energy",sequence?composition_at(sequence_seconds).exposure:1},
        {"geometry_density_multiplier",sequence?composition_at(sequence_seconds).geometry_density_multiplier:1},
        {"previous_exit_progress",sequence?composition_at(sequence_seconds).previous_exit_progress:0},
        {"tunnel_light_weight",sequence?composition_at(sequence_seconds).tunnel_light_weight:1},
        {"average_fps",frame_count/seconds},{"accepted_commands",commands},
        {"mean_delivery_ms",commands?latency_total/commands:0.0},{"max_delivery_ms",latency_max}}.dump()<<std::endl;
    return 0;
}
}
int main(int argc,char** argv) {
    try { return run(options(argc,argv)); }
    catch(const std::exception& e) { std::cerr<<"Error: "<<e.what()<<'\n'; return 1; }
}
