#include "test_support.hpp"
#include "render/time_display.hpp"
#include <limits>
using namespace fantasynt;
int main() {
    // Deteta arredondamento prematuro, deriva do relógio e inversão das coordenadas.
    CHECK(format_time_display(0)=="00:00.0");
    CHECK(format_time_display(59.99)=="00:59.9");
    CHECK(format_time_display(60)=="01:00.0");
    CHECK(format_time_display(127.375)=="02:07.3");
    CHECK(format_time_display(-2)=="00:00.0");
    CHECK(format_time_display(std::numeric_limits<double>::quiet_NaN())=="00:00.0");
    CHECK(format_time_display(std::numeric_limits<double>::infinity())=="00:00.0");
    CHECK(format_time_display(100000)=="99:59.9");
    TimeDisplay display{true,127.375,1.0f,0.5f,0.88f,0.85f};
    const auto desktop=time_display_uniforms(display,1920,1080);
    const auto eye=time_display_uniforms(display,1024,1024);
    CHECK_NEAR(desktop[0][0],1273.0f,0.0f);
    CHECK_NEAR(desktop[0][1],1.0f,0.0f);
    CHECK_NEAR(desktop[0][2],1920.0f/1080.0f,0.00001f);
    CHECK_NEAR(eye[0][2],1.0f,0.0f);
    CHECK_NEAR(desktop[1][0],0.5f,0.0f);
    CHECK_NEAR(desktop[1][1],0.88f,0.0f);
    CHECK(desktop[1]==eye[1]);
    CHECK(time_display_uniforms(display,1920,1080)==desktop);
    display.visible=false;
    const auto hidden=time_display_uniforms(display,1920,1080);
    CHECK_NEAR(hidden[0][1],0.0f,0.0f);
    display.visible=true; display.scale=0; display.opacity=0;
    const auto transparent=time_display_uniforms(display,0,0);
    CHECK(std::isfinite(transparent[0][2]));
    CHECK(transparent[1][2]>0);
    CHECK_NEAR(transparent[1][3],0.0f,0.0f);
    return test_finish();
}
