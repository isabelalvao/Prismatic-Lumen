#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "render/renderer.hpp"
#include "render/art_direction.hpp"
#include "test_support.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
#include <iomanip>
#include <sstream>
#include <bimg/decode.h>
#include <bx/allocator.h>

using namespace fantasynt;
bool same_pixels(const std::vector<char>& a,const std::vector<char>& b) {
    bx::DefaultAllocator allocator;
    auto* first=bimg::imageParse(&allocator,a.data(),static_cast<std::uint32_t>(a.size()),bimg::TextureFormat::RGBA8);
    auto* second=bimg::imageParse(&allocator,b.data(),static_cast<std::uint32_t>(b.size()),bimg::TextureFormat::RGBA8);
    bool same=first&&second&&first->m_width==second->m_width&&first->m_height==second->m_height;
    if(same) {
        const auto* x=static_cast<const unsigned char*>(first->m_data);
        const auto* y=static_cast<const unsigned char*>(second->m_data);
        std::uint64_t difference{};
        const auto size=first->m_width*first->m_height*4;
        for(std::uint32_t i=0;i<size;++i) {
            const auto delta=std::abs(static_cast<int>(x[i])-static_cast<int>(y[i]));
            difference+=delta; same&=delta<=1;
        }
        // A GPU pode variar um nível em poucos canais sem alterar a cena.
        same&=static_cast<double>(difference)/size<0.001;
    }
    if(first) bimg::imageFree(first);
    if(second) bimg::imageFree(second);
    return same;
}
bool continuous_pixels(const std::vector<char>& a,const std::vector<char>& b) {
    bx::DefaultAllocator allocator;
    auto* first=bimg::imageParse(&allocator,a.data(),static_cast<std::uint32_t>(a.size()),bimg::TextureFormat::RGBA8);
    auto* second=bimg::imageParse(&allocator,b.data(),static_cast<std::uint32_t>(b.size()),bimg::TextureFormat::RGBA8);
    bool close=first&&second&&first->m_width==second->m_width&&first->m_height==second->m_height;
    if(close) {
        const auto* x=static_cast<const unsigned char*>(first->m_data);
        const auto* y=static_cast<const unsigned char*>(second->m_data);
        const auto pixels=first->m_width*first->m_height;
        std::uint64_t difference{},outliers{};
        for(std::uint32_t i=0;i<pixels;++i) {
            int maximum{};
            for(int channel=0;channel<3;++channel) {
                const auto delta=std::abs(static_cast<int>(x[i*4+channel])-static_cast<int>(y[i*4+channel]));
                difference+=delta; maximum=std::max(maximum,delta);
            }
            outliers+=maximum>2;
        }
        // Uma variação subpixel pode afetar poucos contornos, mas não uma região inteira.
        close=static_cast<double>(difference)/(pixels*3)<0.02 && static_cast<double>(outliers)/pixels<0.005;
    }
    if(first) bimg::imageFree(first);
    if(second) bimg::imageFree(second);
    return close;
}
bool overlay_at(const std::vector<char>& baseline,const std::vector<char>& overlay,float center_x,float center_y) {
    bx::DefaultAllocator allocator;
    auto* first=bimg::imageParse(&allocator,baseline.data(),static_cast<std::uint32_t>(baseline.size()),bimg::TextureFormat::RGBA8);
    auto* second=bimg::imageParse(&allocator,overlay.data(),static_cast<std::uint32_t>(overlay.size()),bimg::TextureFormat::RGBA8);
    bool localized=first&&second&&first->m_width==second->m_width&&first->m_height==second->m_height;
    std::uint32_t changed{};
    if(localized) {
        const auto* x=static_cast<const unsigned char*>(first->m_data);
        const auto* y=static_cast<const unsigned char*>(second->m_data);
        for(std::uint32_t row=0;row<first->m_height;++row) {
            for(std::uint32_t column=0;column<first->m_width;++column) {
                const auto offset=(row*first->m_width+column)*4;
                int difference{};
                for(int channel=0;channel<3;++channel)
                    difference=std::max(difference,std::abs(static_cast<int>(x[offset+channel])-static_cast<int>(y[offset+channel])));
                if(difference<=3) continue;
                ++changed;
                localized&=std::abs(static_cast<float>(column)/first->m_width-center_x)<0.11f;
                localized&=std::abs(static_cast<float>(row)/first->m_height-center_y)<0.075f;
            }
        }
    }
    if(first) bimg::imageFree(first);
    if(second) bimg::imageFree(second);
    return localized&&changed>20;
}
int main(int argc,char** argv) {
    if(argc!=3&&argc!=4) return 2;
    if(!SDL_Init(SDL_INIT_VIDEO)) return 2;
    struct SDLGuard { ~SDLGuard() { SDL_Quit(); } } sdl_guard;
    int result=1;
    try {
        std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("Fantasynt resize test",640,360,SDL_WINDOW_HIDDEN),SDL_DestroyWindow);
        if(!window) throw std::runtime_error(SDL_GetError());
        auto handle=SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr);
        Renderer renderer(handle,640,360,argv[1]);
        Parameters state;
        if(argc==4&&std::string(argv[3])=="preview") {
            const auto root=std::filesystem::path(argv[2]);
            for(int clip=0;clip<2;++clip) {
                const auto folder=root/(clip==0?"intro-frames":"pyramid-frames");
                std::filesystem::create_directories(folder);
                const int count=clip==0?144:120;
                const double start_time=clip==0?34:78;
                const double fps=clip==0?12:10;
                const double camera=directed_camera_position(clip==0?38:85);
                for(int frame=0;frame<count;++frame) {
                    const double time=start_time+frame/fps;
                    renderer.set_composition(sample_opening(time));
                    for(int stable=0;stable<3;++stable) renderer.draw(state,time,camera);
                    std::ostringstream name; name<<std::setw(4)<<std::setfill('0')<<frame<<".png";
                    renderer.request_capture(folder/name.str());
                    for(int attempt=0;attempt<100&&!renderer.capture_complete();++attempt) {
                        renderer.draw(state,time,camera); SDL_Delay(2);
                    }
                    CHECK(renderer.capture_complete());
                }
            }
            std::cout<<"Preview frames saved to "<<root<<'\n';
            return test_finish();
        }
        renderer.draw(state,0,0);
        renderer.resize(0,0);
        renderer.draw(state,0.1,0);
        renderer.resize(800,450);
        renderer.draw(state,0.2,0);
        renderer.resize(640,360);
        renderer.request_capture(argv[2]);
        for(int i=0;i<100&&!renderer.capture_complete();++i) {
            renderer.draw(state,0.3,0);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        CHECK(renderer.capture_complete());
        std::ifstream image(argv[2],std::ios::binary);
        std::array<unsigned char,24> header{};
        image.read(reinterpret_cast<char*>(header.data()),header.size());
        CHECK(image.good());
        CHECK(header[0]==137 && header[1]=='P' && header[2]=='N' && header[3]=='G');
        auto dimension=[&](std::size_t offset) {
            return (static_cast<std::uint32_t>(header[offset])<<24)|(static_cast<std::uint32_t>(header[offset+1])<<16)|
                (static_cast<std::uint32_t>(header[offset+2])<<8)|header[offset+3];
        };
        CHECK(dimension(16)==640 && dimension(20)==360);
        const auto directory=std::filesystem::path(argv[2]).parent_path()/"controls";
        auto capture=[&](const char* name,const Parameters& parameters,double camera=0.0,double time=0.125) {
            const auto path=directory/(std::string(name)+".png");
            // A captura refere-se ao frame anterior; estabilizar primeiro o estado.
            for(int i=0;i<3;++i) renderer.draw(parameters,time,camera);
            renderer.request_capture(path);
            for(int i=0;i<100&&!renderer.capture_complete();++i) {
                renderer.draw(parameters,time,camera);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            CHECK(renderer.capture_complete());
            std::ifstream file(path,std::ios::binary);
            return std::vector<char>((std::istreambuf_iterator<char>(file)),{});
        };
        const auto reference=capture("baseline",state);
        CHECK(reference.size()>1000);
        renderer.set_time_display({true,127.375});
        const auto timer=capture("timer-visible",state);
        CHECK(!same_pixels(timer,reference));
        CHECK(overlay_at(reference,timer,0.5f,0.88f));
        CHECK(same_pixels(capture("timer-frozen",state),timer));
        renderer.set_time_display({true,128.0});
        CHECK(!same_pixels(capture("timer-next-second",state),timer));
        renderer.set_time_display({true,127.375,1.0f,0.2f,0.25f,0.85f});
        CHECK(overlay_at(reference,capture("timer-position",state),0.2f,0.25f));
        renderer.set_time_display({true,127.375,1.0f,0.5f,0.88f,0.0f});
        CHECK(same_pixels(capture("timer-transparent",state),reference));
        renderer.set_time_display({false,128.0});
        CHECK(same_pixels(capture("timer-hidden",state),reference));
        auto changed=state; changed.light_intensity=0;
        CHECK(capture("light-zero",changed)!=reference);
        changed=state; changed.palette={Color{1,0.01f,0.01f},Color{1,0.01f,0.01f}};
        CHECK(capture("palette-red",changed)!=reference);
        changed=state; changed.pulse_strength=1;
        CHECK(capture("pulse-max",changed)!=reference);
        changed=state; changed.fog_density=1;
        CHECK(capture("fog-max",changed)!=reference);
        changed=state; changed.geometry_density=0;
        CHECK(capture("density-zero",changed)!=reference);
        changed=state; changed.bloom=1;
        CHECK(capture("bloom-max",changed)!=reference);
        CHECK(capture("camera-advanced",state,500)!=reference);
        renderer.set_scene_layout(SceneLayout::Field);
        CHECK(capture("legacy-field",state)!=reference);
        renderer.set_scene_layout(SceneLayout::Night);
        const auto night=capture("night",state);
        CHECK(night!=reference);
        renderer.set_scene_layout(SceneLayout::Landscape);
        const auto landscape=capture("landscape",state);
        CHECK(landscape!=night && landscape!=reference);
        renderer.set_composition(sample_sequence(96));
        CHECK(capture("night-landscape-transition",state)!=landscape);
        renderer.set_composition(sample_sequence(228));
        CHECK(capture("landscape-tunnel-transition",state)!=landscape);
        CHECK(sample_sequence(231.99).weights[2]<1);
        renderer.set_composition(sample_sequence(231.99));
        const auto before_tunnel=capture("tunnel-boundary-before",state);
        renderer.set_composition(sample_sequence(232));
        CHECK(continuous_pixels(before_tunnel,capture("tunnel-boundary-after",state)));
        renderer.set_scene_layout(SceneLayout::Tunnel);
        CHECK(same_pixels(capture("tunnel-restored",state),reference));
        renderer.set_composition(sample_opening(40.125));
        const double opening_camera=directed_camera_position(40.125);
        const auto opening=capture("opening-baseline",state,opening_camera,40.125);
        changed=state; changed.pulse_strength=0;
        CHECK(!same_pixels(capture("opening-pulse-zero",changed,opening_camera,40.125),opening));
        changed=state; changed.palette[1]=Color{1,0.01f,0.01f};
        CHECK(!same_pixels(capture("opening-second-colour",changed,opening_camera,40.125),opening));
        changed=state; changed.geometry_density=0;
        CHECK(!same_pixels(capture("opening-density-zero",changed,opening_camera,40.125),opening));
        double quiet_time=40.125;
        while(quiet_time<60&&renderer.beat_strength(quiet_time)>0) quiet_time+=0.01;
        CHECK(quiet_time<60);
        renderer.set_composition(sample_opening(quiet_time));
        const auto quiet=capture("opening-between-beats",state,opening_camera,quiet_time);
        changed=state; changed.pulse_strength=0;
        CHECK(same_pixels(capture("opening-between-beats-zero",changed,opening_camera,quiet_time),quiet));
        const auto faster_points=parse_visual_config(R"({"schema_version":1,"points":{"fall_speed_m_s":1.0}})");
        renderer.set_visual_config(faster_points);
        CHECK(same_pixels(capture("opening-live-speed-continuity",state,opening_camera,quiet_time),quiet));
        const auto red_traces=parse_visual_config(R"({"schema_version":1,"points":{"fall_speed_m_s":1.0},"families":{"trace":{"colour":"#FF0000"}}})");
        renderer.set_visual_config(red_traces);
        CHECK(!same_pixels(capture("opening-live-trace-colour",state,opening_camera,quiet_time),quiet));
        renderer.set_visual_config(default_visual_config());
        renderer.set_composition(sample_sequence(127.9999));
        const auto before_handoff=capture("opening-handoff-before",state,opening_camera,127.9999);
        renderer.set_composition(sample_sequence(128));
        CHECK(continuous_pixels(before_handoff,capture("opening-handoff-after",state,opening_camera,128)));
        auto steady=state; steady.pulse_strength=0;
        const auto frozen=parse_visual_config(R"({"schema_version":1,"animations":{"speed_multiplier":0}})");
        renderer.set_visual_config(frozen);
        for(double boundary:{136.0,172.0,224.0}) {
            const auto position=directed_camera_position(boundary,frozen);
            const auto phase_name=std::to_string(static_cast<int>(boundary));
            renderer.set_composition(directed_composition(boundary,true,frozen));
            const auto before=capture(("phase-"+phase_name+"-before").c_str(),directed_visual_parameters(steady,boundary,frozen),position,boundary);
            renderer.set_composition(directed_composition(boundary+0.01,true,frozen));
            CHECK(continuous_pixels(before,capture(("phase-"+phase_name+"-after").c_str(),directed_visual_parameters(steady,boundary+0.01,frozen),position,boundary+0.01)));
        }
        renderer.set_visual_config(default_visual_config());
        renderer.set_composition(directed_composition(119,true,default_visual_config()));
        const auto near_end=capture("unlit-enabled",steady,directed_camera_position(119),119);
        const auto unlit_off=parse_visual_config(R"({"schema_version":1,"materials":{"unlit_pattern_emission":0}})");
        renderer.set_visual_config(unlit_off);
        CHECK(!same_pixels(near_end,capture("unlit-disabled",steady,directed_camera_position(119),119)));
        renderer.set_visual_config(default_visual_config());
        const auto pole_reference=capture("pole-before-reload",steady,opening_camera,40.125);
        const auto pole_variation=parse_visual_config(R"({"schema_version":1,"poles":{"period_variation":0.6}})");
        renderer.set_visual_config(pole_variation);
        CHECK(same_pixels(pole_reference,capture("pole-after-reload",steady,opening_camera,40.125)));
        result=test_finish();
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; }
    return result;
}
