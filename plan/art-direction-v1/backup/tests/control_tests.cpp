#include "control/protocol.hpp"
#include "control/scene_controller.hpp"
#include "test_support.hpp"
#include <nlohmann/json.hpp>
#include <limits>

using namespace fantasynt;
const std::string valid = R"({"schema_version":1,"id":"test-1","command":"apply_parameters","transition_seconds":2,"parameters":{"light_intensity":0.8}})";

void accepts_and_normalizes() {
    auto parsed = parse_command(valid);
    CHECK(std::holds_alternative<Command>(parsed));
    if (auto c = std::get_if<Command>(&parsed)) {
        CHECK(c->id == "test-1");
        CHECK_NEAR(c->parameters.light_intensity.value(), 0.8f, 0.00001f);
        CHECK(!c->parameters.fog_density);
    }
    auto data = nlohmann::json::parse(valid);
    data["parameters"] = {{"palette", {"#808080", "#FFFFFF"}}};
    parsed = parse_command(data.dump());
    CHECK(std::holds_alternative<Command>(parsed));
    if (auto c = std::get_if<Command>(&parsed)) {
        CHECK_NEAR(c->parameters.palette.value()[0].r, 0.215861f, 0.00001f);
        CHECK_NEAR(c->parameters.palette.value()[1].b, 1.0f, 0.00001f);
    }
}
void rejects_invalid_input() {
    const std::vector<std::string> invalid = {
        "", "[]", "null", "{",
        R"({"schema_version":1,"schema_version":1})",
        R"({"schema_version":1,"id":"a","command":"apply_parameters","transition_seconds":2,"parameters":{"bloom":0.1,"bloom":0.2}})",
        std::string(max_message_bytes + 1, ' '),
        std::string("{\"id\":\"") + char(0xff) + "\"}"
    };
    for (const auto& text : invalid) CHECK(std::holds_alternative<ValidationError>(parse_command(text)));
    auto base = nlohmann::json::parse(valid);
    const std::vector<nlohmann::json> bad_values = {true, false, "0.5", nullptr, -0.1, 1.1, {}, nlohmann::json::array()};
    for (const auto& value : bad_values) {
        auto data = base;
        data["parameters"]["light_intensity"] = value;
        CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    }
    for (const auto* key : {"schema_version", "id", "command", "transition_seconds", "parameters"}) {
        auto data = base; data.erase(key);
        CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    }
    for (auto value : {-1.0, 0.0, 0.099, 15.01}) {
        auto data=base; data["transition_seconds"]=value;
        CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    }
    for (const auto& id : {std::string(), std::string(65,'a'), std::string("bad id"), std::string("ação")}) {
        auto data=base; data["id"]=id;
        CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    }
    auto data=base; data["schema_version"]=1.0;
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    data=base; data["extra"]=1;
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    data=base; data["parameters"]={{"exec","anything"}};
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    data=base; data["parameters"]=nlohmann::json::object();
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    data=base; data["parameters"]={{"palette",{"#ABCDEF"}}};
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    data=base; data["parameters"]={{"palette",{"#GGGGGG","#FFFFFF"}}};
    CHECK(std::holds_alternative<ValidationError>(parse_command(data.dump())));
    CHECK(std::holds_alternative<ValidationError>(parse_command(R"({"schema_version":1,"id":"a","command":"apply_parameters","transition_seconds":1e999,"parameters":{"bloom":0.3}})")));
}
void accepts_boundaries() {
    for (auto duration : {0.1,15.0}) {
        auto data=nlohmann::json::parse(valid);
        data["transition_seconds"]=duration;
        data["parameters"]={{"light_intensity",0},{"motion_speed",1},{"pulse_strength",0},{"fog_density",1},{"geometry_density",0},{"bloom",1}};
        CHECK(std::holds_alternative<Command>(parse_command(data.dump())));
    }
    auto padded=valid + std::string(max_message_bytes-valid.size(),' ');
    CHECK(std::holds_alternative<Command>(parse_command(padded)));
}
void transitions_are_continuous_and_partial() {
    SceneController scene;
    Command first{"first",2.0,{}};
    first.parameters.light_intensity=0.0f;
    first.parameters.motion_speed=0.9f;
    scene.submit(first);
    CHECK(scene.revision()==1);
    scene.advance(1.0);
    CHECK_NEAR(scene.current().light_intensity,0.3f,0.00001f);
    auto mid=scene.current().light_intensity;
    Command second{"second",2.0,{}};
    second.parameters.light_intensity=1.0f;
    scene.submit(second);
    CHECK_NEAR(scene.current().light_intensity,mid,0.00001f);
    CHECK_NEAR(scene.target().motion_speed,0.9f,0.00001f);
    CHECK_NEAR(scene.target().fog_density,0.2f,0.00001f);
    scene.advance(-1.0);
    scene.advance(std::numeric_limits<double>::infinity());
    CHECK_NEAR(scene.current().light_intensity,mid,0.00001f);
    scene.advance(2.0);
    CHECK_NEAR(scene.current().light_intensity,1.0f,0.00001f);
    CHECK_NEAR(scene.current().motion_speed,0.9f,0.00001f);
    CHECK(scene.revision()==2);
    SceneController one, many;
    one.submit(first); many.submit(first);
    one.advance(1.0);
    for(int i=0;i<100;++i) many.advance(0.01);
    CHECK_NEAR(one.current().light_intensity,many.current().light_intensity,0.00001f);
}
int main() {
    accepts_and_normalizes();
    rejects_invalid_input();
    accepts_boundaries();
    transitions_are_continuous_and_partial();
    return test_finish();
}
