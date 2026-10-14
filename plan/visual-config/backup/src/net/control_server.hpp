#pragma once
#include <cstdint>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace fantasynt {
struct PendingMessage {
    std::uint64_t client_id{};
    std::string payload;
    std::chrono::steady_clock::time_point received;
};
class ControlServer {
public:
    explicit ControlServer(std::uint16_t port);
    ~ControlServer();
    ControlServer(const ControlServer&) = delete;
    ControlServer& operator=(const ControlServer&) = delete;
    std::uint16_t port() const;
    std::vector<PendingMessage> drain();
    void reply(std::uint64_t client_id, std::string payload);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
