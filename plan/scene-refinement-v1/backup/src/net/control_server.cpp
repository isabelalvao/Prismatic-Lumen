#include <winsock2.h>
#include <ws2tcpip.h>
#include "control_server.hpp"
#include "line_framer.hpp"
#include <algorithm>
#include <array>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <utility>
#include <nlohmann/json.hpp>

namespace fantasynt {
namespace {
struct Winsock {
    Winsock() {
        WSADATA data{};
        if (const int code = WSAStartup(MAKEWORD(2, 2), &data); code != 0)
            throw std::runtime_error("WSAStartup failed: " + std::to_string(code));
    }
    ~Winsock() { WSACleanup(); }
};
struct Socket {
    SOCKET value{INVALID_SOCKET};
    Socket() = default;
    explicit Socket(SOCKET s) : value(s) {}
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket(Socket&& other) noexcept : value(std::exchange(other.value, INVALID_SOCKET)) {}
    Socket& operator=(Socket&& other) noexcept {
        if (this != &other) { close(); value = std::exchange(other.value, INVALID_SOCKET); }
        return *this;
    }
    ~Socket() { close(); }
    void close() { if (value != INVALID_SOCKET) closesocket(std::exchange(value, INVALID_SOCKET)); }
};
void nonblocking(SOCKET socket) {
    u_long enabled = 1;
    if (ioctlsocket(socket, FIONBIO, &enabled) != 0) throw std::runtime_error("Cannot set nonblocking socket");
}
std::string transport_error(const char* code) {
    return nlohmann::json{{"schema_version", 1}, {"status", "rejected"}, {"error_code", code}}.dump() + '\n';
}
struct Client {
    Socket socket;
    std::uint64_t id{};
    LineFramer framer;
    std::string output;
    std::size_t sent{};
};
}

struct ControlServer::Impl {
    Winsock winsock;
    Socket listener;
    std::uint16_t bound_port{};
    std::uint64_t next_client{1};
    std::vector<Client> clients;
    std::mutex mutex;
    std::deque<PendingMessage> incoming;
    std::deque<std::pair<std::uint64_t, std::string>> outgoing;
    std::unordered_set<std::uint64_t> close_clients;
    std::string failure;
    std::jthread worker;

    explicit Impl(std::uint16_t port) {
        listener = Socket(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        if (listener.value == INVALID_SOCKET) throw std::runtime_error("Cannot create listener");
        BOOL exclusive = TRUE;
        if (setsockopt(listener.value, SOL_SOCKET, SO_EXCLUSIVEADDRUSE,
            reinterpret_cast<const char*>(&exclusive), sizeof(exclusive)) != 0)
            throw std::runtime_error("Cannot configure listener");
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(port);
        if (bind(listener.value, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0)
            throw std::runtime_error("Cannot bind local port " + std::to_string(port) + ": " + std::to_string(WSAGetLastError()));
        if (listen(listener.value, 8) != 0) throw std::runtime_error("Cannot listen");
        int length = sizeof(address);
        if (getsockname(listener.value, reinterpret_cast<sockaddr*>(&address), &length) != 0)
            throw std::runtime_error("Cannot resolve bound port");
        bound_port = ntohs(address.sin_port);
        nonblocking(listener.value);
        worker = std::jthread([this](std::stop_token stop) {
            try { run(stop); }
            catch (const std::exception& e) { std::lock_guard lock(mutex); failure = e.what(); }
        });
    }
    ~Impl() { worker.request_stop(); if (worker.joinable()) worker.join(); }

    void append(Client& client, const std::string& bytes) {
        if (client.sent) { client.output.erase(0, client.sent); client.sent = 0; }
        if (client.output.size() + bytes.size() > 64 * 1024) client.socket.close();
        else client.output += bytes;
    }
    void transfer_replies() {
        std::deque<std::pair<std::uint64_t, std::string>> replies;
        std::unordered_set<std::uint64_t> closes;
        {
            std::lock_guard lock(mutex);
            replies.swap(outgoing); closes.swap(close_clients);
        }
        for (auto& client : clients) if (closes.contains(client.id)) client.socket.close();
        for (const auto& [id, data] : replies) {
            auto client = std::find_if(clients.begin(), clients.end(), [id](const auto& c) { return c.id == id; });
            if (client != clients.end() && client->socket.value != INVALID_SOCKET) append(*client, data);
        }
    }
    void read_client(Client& client) {
        std::array<char, 8192> buffer{};
        const int count = recv(client.socket.value, buffer.data(), static_cast<int>(buffer.size()), 0);
        if (count <= 0) {
            if (count == 0 || WSAGetLastError() != WSAEWOULDBLOCK) client.socket.close();
            return;
        }
        for (auto& frame : client.framer.feed(std::string_view(buffer.data(), count))) {
            if (frame.oversized) { append(client, transport_error("message_too_large")); continue; }
            bool full;
            {
                std::lock_guard lock(mutex);
                full = incoming.size() >= 64;
                if (!full) incoming.push_back({client.id, std::move(frame.payload.value()), std::chrono::steady_clock::now()});
            }
            if (full) append(client, transport_error("queue_full"));
        }
    }
    void write_client(Client& client) {
        const auto remaining = client.output.size() - client.sent;
        const int count = send(client.socket.value, client.output.data() + client.sent,
            static_cast<int>(remaining), 0);
        if (count > 0) {
            client.sent += static_cast<std::size_t>(count);
            if (client.sent == client.output.size()) { client.output.clear(); client.sent = 0; }
        } else if (count == 0 || WSAGetLastError() != WSAEWOULDBLOCK) client.socket.close();
    }
    void run(std::stop_token stop) {
        while (!stop.stop_requested()) {
            transfer_replies();
            std::erase_if(clients, [](const auto& c) { return c.socket.value == INVALID_SOCKET; });
            fd_set readable{}, writable{};
            FD_ZERO(&readable); FD_ZERO(&writable); FD_SET(listener.value, &readable);
            for (auto& client : clients) {
                FD_SET(client.socket.value, &readable);
                if (!client.output.empty()) FD_SET(client.socket.value, &writable);
            }
            timeval timeout{0, 10000};
            const int ready = select(0, &readable, &writable, nullptr, &timeout);
            if (ready == SOCKET_ERROR) throw std::runtime_error("Local server select failed");
            if (FD_ISSET(listener.value, &readable)) {
                Socket accepted(accept(listener.value, nullptr, nullptr));
                if (accepted.value != INVALID_SOCKET && clients.size() < 8) {
                    nonblocking(accepted.value);
                    clients.push_back({std::move(accepted), next_client++, {}, {}, 0});
                }
            }
            for (auto& client : clients) {
                if (FD_ISSET(client.socket.value, &readable)) read_client(client);
                if (client.socket.value != INVALID_SOCKET && FD_ISSET(client.socket.value, &writable)) write_client(client);
            }
        }
    }
};
ControlServer::ControlServer(std::uint16_t port) : impl_(std::make_unique<Impl>(port)) {}
ControlServer::~ControlServer() = default;
std::uint16_t ControlServer::port() const { return impl_->bound_port; }
std::vector<PendingMessage> ControlServer::drain() {
    std::lock_guard lock(impl_->mutex);
    if (!impl_->failure.empty()) throw std::runtime_error(impl_->failure);
    std::vector<PendingMessage> result;
    result.reserve(impl_->incoming.size());
    while (!impl_->incoming.empty()) {
        result.push_back(std::move(impl_->incoming.front())); impl_->incoming.pop_front();
    }
    return result;
}
void ControlServer::reply(std::uint64_t client_id, std::string payload) {
    std::lock_guard lock(impl_->mutex);
    if (impl_->outgoing.size() >= 64) impl_->close_clients.insert(client_id);
    else impl_->outgoing.emplace_back(client_id, std::move(payload) + '\n');
}
}
