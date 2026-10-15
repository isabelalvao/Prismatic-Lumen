#pragma once
#include "motif_curve.hpp"
#include "control/parameters.hpp"
#include <nlohmann/json.hpp>
#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

namespace fantasynt::behavior {
struct EffectiveState {
    Parameters directed,effective;
    double envelope{},light_contribution{},gain{1};
    bool clamped{};
};
class MotifExecutor {
public:
    explicit MotifExecutor(bool enabled,MotifDose dose={}):enabled_(enabled),dose_(validated_dose(dose)) {}
    void restart(double seconds,bool paused,std::string_view reason="restart");
    void advance(double seconds,bool paused,std::string_view reason="operator_pause");
    void stop(std::string_view reason);
    EffectiveState compose(const Parameters& directed) const;
    nlohmann::json state() const;
    std::vector<nlohmann::json> drain_events();
private:
    enum class Status { Pending,Running,Completed,Cancelled,Missed };
    void emit(std::string_view event,std::size_t i,std::string_view reason={});
    bool enabled_{},initialized_{},paused_{},stopped_{};
    MotifDose dose_;
    double time_{};
    std::uint64_t epoch_{};
    std::array<Status,agenda.size()> status_{};
    std::array<bool,agenda.size()> applied_{};
    std::vector<nlohmann::json> events_;
};
}
