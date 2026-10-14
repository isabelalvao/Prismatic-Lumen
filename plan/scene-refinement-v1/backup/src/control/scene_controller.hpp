#pragma once
#include "parameters.hpp"

namespace fantasynt {
class SceneController {
public:
    SceneController() = default;
    explicit SceneController(const Parameters& initial):current_(initial),target_(initial),origin_(initial) {}
    void submit(const Command& command);
    void advance(double elapsed_seconds);
    const Parameters& current() const { return current_; }
    const Parameters& target() const { return target_; }
    std::uint64_t revision() const { return revision_; }
private:
    Parameters current_, target_, origin_;
    double elapsed_{}, duration_{};
    std::uint64_t revision_{};
};
}
