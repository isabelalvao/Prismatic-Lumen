#pragma once
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace fantasynt {
struct FrameResult { std::optional<std::string> payload; bool oversized{}; };
class LineFramer {
public:
    std::vector<FrameResult> feed(std::string_view bytes);
private:
    std::string buffer_;
    bool discarding_{};
};
}
