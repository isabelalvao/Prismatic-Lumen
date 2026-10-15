#include "audio/player.hpp"
#include "test_support.hpp"
#include <limits>
#include <stdexcept>

using namespace fantasynt;
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    try {
        AudioPlayer player(std::filesystem::u8path(argv[1]));
        CHECK_NEAR(player.duration(),424.1821315192744,0.000001);
        player.seek(180);
        CHECK_NEAR(player.position(),180,0.00001);
        player.set_paused(false); SDL_Delay(100); player.set_paused(true);
        const auto frozen=player.position();
        CHECK(frozen>180 && frozen<180.3);
        SDL_Delay(70); CHECK(player.position()==frozen);
        player.seek(0); CHECK(player.position()==0);
        player.seek(-5); CHECK(player.position()==0);
        player.seek(500); CHECK_NEAR(player.position(),player.duration(),0.00003);
        bool rejected=false;
        try { player.seek(std::numeric_limits<double>::quiet_NaN()); }
        catch(const std::runtime_error&) { rejected=true; }
        CHECK(rejected);
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; SDL_Quit(); return 1; }
    SDL_Quit();
    return test_finish();
}
