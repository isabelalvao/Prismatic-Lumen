#include "config/visual_config.hpp"
#include "render/art_direction.hpp"
#include "test_support.hpp"
#include <fstream>
#include <unordered_set>

using namespace fantasynt;
int main(int argc,char** argv) {
    CHECK(argc==2);
    const auto config=load_visual_config(argv[1]);
    CHECK(config[Setting::AutoPlay]==1 && config[Setting::CameraDelay]==0 && config[Setting::CameraAcceleration]==0);
    CHECK(config[Setting::PyramidStart]==80);
    const auto round_trip=parse_visual_config(visual_config_json(config));
    std::unordered_set<std::string> fields;
    for(const auto& d:visual_setting_definitions()) {
        CHECK(fields.insert(d.path).second);
        CHECK(round_trip[d.id]==config[d.id]);
    }
    CHECK(fields.size()==static_cast<std::size_t>(Setting::Count));
    const auto customized=parse_visual_config(R"({"schema_version":1,"points":{"fall_speed_m_s":0.25},"camera":{"speed_m_s":0.8},"traces":{"direction":"up"},"families":{"pyramids":{"enabled":false}}})");
    CHECK_NEAR(falling_point_height(2000,1,customized),1975,0.001);
    CHECK_NEAR(directed_camera_position(1,customized)-directed_camera_position(0,customized),80,0.001);
    CHECK(customized[Setting::TraceDirection]==1 && customized[Setting::EnablePyramids]==0);
    for(const auto* invalid:{R"({"schema_version":1,"camera":{"speed_m_s":-1}})",
        R"({"schema_version":1,"traces":{"speed_m_s":1,"speed_m_s":2}})",
        R"({"schema_version":1,"post":{"unknown":2}})",
        R"({"schema_version":1,"startup":{"auto_play":1}})",
        R"({"schema_version":1,"sky":{"point_inner_radius":0.4,"point_outer_radius":0.2}})",
        R"({"schema_version":1,"layout":{"intro_sectors":2.5}})",
        R"({"schema_version":1,"families":{"points":{"colour":"bad"}}})"}) {
        bool rejected=false;
        try { (void)parse_visual_config(invalid); } catch(const std::exception&) { rejected=true; }
        CHECK(rejected);
    }
    return test_finish();
}
