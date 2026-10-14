#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "render/renderer.hpp"
#include "test_support.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <thread>
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
int main(int argc,char** argv) {
    if(argc!=3) return 2;
    if(!SDL_Init(SDL_INIT_VIDEO)) return 2;
    int result=1;
    try {
        std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(
            SDL_CreateWindow("Fantasynt resize test",640,360,SDL_WINDOW_HIDDEN),SDL_DestroyWindow);
        if(!window) throw std::runtime_error(SDL_GetError());
        auto handle=SDL_GetPointerProperty(SDL_GetWindowProperties(window.get()),SDL_PROP_WINDOW_WIN32_HWND_POINTER,nullptr);
        Renderer renderer(handle,640,360,argv[1]);
        Parameters state;
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
        auto capture=[&](const char* name,const Parameters& parameters,double camera=0.0) {
            const auto path=directory/(std::string(name)+".png");
            // A captura refere-se ao frame anterior; estabilizar primeiro o estado.
            for(int i=0;i<3;++i) renderer.draw(parameters,0.125,camera);
            renderer.request_capture(path);
            for(int i=0;i<100&&!renderer.capture_complete();++i) {
                renderer.draw(parameters,0.125,camera);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            CHECK(renderer.capture_complete());
            std::ifstream file(path,std::ios::binary);
            return std::vector<char>((std::istreambuf_iterator<char>(file)),{});
        };
        const auto reference=capture("baseline",state);
        CHECK(reference.size()>1000);
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
        renderer.set_scene_layout(SceneLayout::Tunnel);
        CHECK(same_pixels(capture("tunnel-restored",state),reference));
        result=test_finish();
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; }
    SDL_Quit();
    return result;
}
