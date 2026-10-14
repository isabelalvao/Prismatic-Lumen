#include "line_framer.hpp"
#include "control/parameters.hpp"
namespace fantasynt {
std::vector<FrameResult> LineFramer::feed(std::string_view bytes) {
    std::vector<FrameResult> result;
    for (char c : bytes) {
        if (discarding_) {
            if (c == '\n') discarding_ = false;
            continue;
        }
        if (c == '\n') {
            if (!buffer_.empty() && buffer_.back() == '\r') buffer_.pop_back();
            result.push_back({std::move(buffer_), false});
            buffer_.clear();
        } else {
            buffer_.push_back(c);
            const bool possible_crlf = buffer_.size() == max_message_bytes + 1 && c == '\r';
            if (buffer_.size() > max_message_bytes && !possible_crlf) {
                buffer_.clear();
                discarding_ = true;
                result.push_back({{}, true});
            }
        }
    }
    return result;
}
}
