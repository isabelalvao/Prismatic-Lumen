#pragma once
#include <nlohmann/json.hpp>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

namespace fantasynt {
struct RecordingStats {
    std::uint64_t accepted{},dropped{};
    bool write_failed{},complete{};
};
// Um produtor (ciclo principal), um escritor; submit nunca espera por disco/mutex.
class SessionRecorder {
public:
    SessionRecorder(const std::filesystem::path& path,nlohmann::json manifest,std::size_t capacity=256);
    ~SessionRecorder();
    SessionRecorder(const SessionRecorder&)=delete;
    SessionRecorder& operator=(const SessionRecorder&)=delete;
    bool submit(nlohmann::json event);
    RecordingStats close(bool normal_exit);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
std::string file_sha256(const std::filesystem::path& path);
std::string new_session_id();
std::string path_utf8(const std::filesystem::path& path);
}
