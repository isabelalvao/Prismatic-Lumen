#include "test_support.hpp"
#include "render/time_display.hpp"
#include <limits>
using namespace fantasynt;
namespace {
using Matrix=std::array<float,16>;
Matrix identity() { return {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}; }
Matrix projection(float horizontal,float vertical,float offset_x,float offset_y) {
    return {horizontal,0,0,0,0,vertical,0,0,offset_x,offset_y,1,1,0,0,-10,0};
}
std::array<float,4> transform(const Matrix& matrix,std::array<float,4> point) {
    std::array<float,4> result{};
    for(int row=0;row<4;++row)
        for(int column=0;column<4;++column) result[row]+=matrix[column*4+row]*point[column];
    return result;
}
std::array<float,2> project(const Matrix& projection,const Matrix& head_to_eye,std::array<float,4> point) {
    const auto clip=transform(projection,transform(head_to_eye,point));
    return {0.5f+0.5f*clip[0]/clip[3],0.5f-0.5f*clip[1]/clip[3]};
}
std::array<float,2> glyph_position(const TimeDisplayUniforms& uniforms,std::array<float,2> uv) {
    std::array<float,3> mapped{};
    for(int row=0;row<3;++row)
        mapped[row]=uniforms[2+row][0]*uv[0]+uniforms[2+row][1]*uv[1]+uniforms[2+row][2];
    return {mapped[0]/mapped[2],mapped[1]/mapped[2]};
}
}
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
    CHECK(!TimeDisplay{}.visible);
    CHECK_NEAR(time_display_uniforms(TimeDisplay{},1920,1080)[0][1],0.0f,0.0f);
    display={true,127.375,1.0f,0.5f,0.65f,0.85f};
    const auto desktop_center=glyph_position(time_display_uniforms(display,1920,1080),{display.x,display.y});
    CHECK_NEAR(desktop_center[0],1.82f,0.0001f);
    CHECK_NEAR(desktop_center[1],0.5f,0.0001f);
    // A mesma placa física deve projetar-se de forma diferente nos dois olhos.
    const auto symmetric=projection(1,1,0,0);
    auto left_view=identity(),right_view=identity();
    left_view[12]=3.2f; right_view[12]=-3.2f;
    const auto left=time_display_uniforms_vr(display,symmetric,left_view);
    const auto right=time_display_uniforms_vr(display,symmetric,right_view);
    const std::array<float,4> center{0,-60,200,1};
    const auto left_uv=project(symmetric,left_view,center),right_uv=project(symmetric,right_view,center);
    CHECK_NEAR(left_uv[0]-right_uv[0],0.016f,0.00001f);
    CHECK_NEAR(glyph_position(left,left_uv)[0],1.82f,0.0001f);
    CHECK_NEAR(glyph_position(right,right_uv)[0],1.82f,0.0001f);
    // A disparidade e os raios convergem a 200 cm, inclusive com FOV assimétrico.
    const auto left_projection=projection(0.84f,1.2f,0.14f,-0.09f);
    const auto right_projection=projection(1.12f,0.91f,-0.11f,0.07f);
    const auto asymmetric_left=time_display_uniforms_vr(display,left_projection,left_view);
    const auto asymmetric_right=time_display_uniforms_vr(display,right_projection,right_view);
    const auto asymmetric_left_uv=project(left_projection,left_view,center);
    const auto asymmetric_right_uv=project(right_projection,right_view,center);
    const float left_ray=(2*asymmetric_left_uv[0]-1-left_projection[8])/left_projection[0];
    const float right_ray=(2*asymmetric_right_uv[0]-1-right_projection[8])/right_projection[0];
    CHECK_NEAR(6.4f/(left_ray-right_ray),200.0f,0.001f);
    CHECK_NEAR(glyph_position(asymmetric_left,asymmetric_left_uv)[0],1.82f,0.0001f);
    CHECK_NEAR(glyph_position(asymmetric_right,asymmetric_right_uv)[0],1.82f,0.0001f);
    // Olhos inclinados exigem a homografia completa para fundir também os extremos.
    auto canted=identity();
    const float angle=0.12f;
    canted[0]=std::cos(angle); canted[2]=-std::sin(angle);
    canted[8]=std::sin(angle); canted[10]=std::cos(angle); canted[12]=3.2f;
    const auto canted_uniforms=time_display_uniforms_vr(display,left_projection,canted);
    for(const auto glyph:std::array<std::array<float,2>,3>{{{0,0},{1.82f,0.5f},{3.64f,1}}}) {
        const std::array<float,4> point{(glyph[0]-1.82f)*20.8f,-60-(glyph[1]-0.5f)*20.8f,200,1};
        const auto actual=glyph_position(canted_uniforms,project(left_projection,canted,point));
        CHECK_NEAR(actual[0],glyph[0],0.0001f);
        CHECK_NEAR(actual[1],glyph[1],0.0001f);
    }
    const auto vr_hidden=time_display_uniforms_vr(TimeDisplay{},symmetric,left_view);
    CHECK_NEAR(vr_hidden[0][1],0.0f,0.0f);
    CHECK_NEAR(time_display_uniforms_vr(display,Matrix{},left_view)[0][1],0.0f,0.0f);
    CHECK_NEAR(time_display_uniforms_vr(display,symmetric,left_view,-1)[0][1],0.0f,0.0f);
    return test_finish();
}
