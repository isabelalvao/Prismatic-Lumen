#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#include "session_recorder.hpp"
#include <array>
#include <atomic>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

namespace fantasynt {
namespace {
using Json=nlohmann::json;
std::string hex(const unsigned char* bytes,std::size_t count) {
    constexpr char digits[]="0123456789abcdef";
    std::string result; result.reserve(count*2);
    for(std::size_t i=0;i<count;++i) { result.push_back(digits[bytes[i]>>4]); result.push_back(digits[bytes[i]&15]); }
    return result;
}
struct Algorithm {
    BCRYPT_ALG_HANDLE value{};
    ~Algorithm() { if(value) BCryptCloseAlgorithmProvider(value,0); }
};
struct Hash {
    BCRYPT_HASH_HANDLE value{};
    ~Hash() { if(value) BCryptDestroyHash(value); }
};
void crypto_checked(NTSTATUS result) { if(result<0) throw std::runtime_error("Session SHA-256 operation failed"); }
}
std::string path_utf8(const std::filesystem::path& path) {
    const auto bytes=path.u8string();
    return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
}
std::string new_session_id() {
    std::array<unsigned char,16> bytes{};
    crypto_checked(BCryptGenRandom(nullptr,bytes.data(),static_cast<ULONG>(bytes.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG));
    return hex(bytes.data(),bytes.size());
}
std::string file_sha256(const std::filesystem::path& path) {
    std::ifstream input(path,std::ios::binary);
    if(!input) throw std::runtime_error("Cannot hash session resource: "+path_utf8(path));
    Algorithm algorithm;
    crypto_checked(BCryptOpenAlgorithmProvider(&algorithm.value,BCRYPT_SHA256_ALGORITHM,nullptr,0));
    ULONG length{},received{};
    crypto_checked(BCryptGetProperty(algorithm.value,BCRYPT_OBJECT_LENGTH,
        reinterpret_cast<PUCHAR>(&length),sizeof(length),&received,0));
    std::vector<unsigned char> object(length);
    Hash hash;
    crypto_checked(BCryptCreateHash(algorithm.value,&hash.value,object.data(),length,nullptr,0,0));
    std::array<char,65536> buffer{};
    while(input.read(buffer.data(),buffer.size())||input.gcount())
        crypto_checked(BCryptHashData(hash.value,reinterpret_cast<PUCHAR>(buffer.data()),static_cast<ULONG>(input.gcount()),0));
    if(!input.eof()) throw std::runtime_error("Cannot read session resource: "+path_utf8(path));
    std::array<unsigned char,32> digest{};
    crypto_checked(BCryptFinishHash(hash.value,digest.data(),static_cast<ULONG>(digest.size()),0));
    return hex(digest.data(),digest.size());
}
struct SessionRecorder::Impl {
    HANDLE file{INVALID_HANDLE_VALUE};
    std::vector<std::optional<Json>> queue;
    std::atomic<std::size_t> head{},tail{};
    std::atomic<std::uint64_t> accepted{},dropped{},wake{};
    std::atomic<bool> stopping{},write_failed{};
    bool normal_exit{},closed{};
    std::thread writer;
    explicit Impl(const std::filesystem::path& path,Json manifest,std::size_t capacity):queue(capacity+1) {
        if(!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
        // CREATE_NEW impede apagar um registo existente, incluindo na corrida de abertura.
        file=CreateFileW(path.c_str(),GENERIC_WRITE,FILE_SHARE_READ,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file==INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot create new session recording: "+path_utf8(path));
        try {
            manifest["type"]="session_manifest"; manifest["schema_version"]=1;
            writer=std::thread([this,manifest=std::move(manifest)] { write_loop(manifest); });
        } catch(...) { CloseHandle(file); throw; }
    }
    bool write_json(const Json& event) noexcept {
        try {
            const auto bytes=event.dump()+"\n";
            std::size_t offset{};
            while(offset<bytes.size()) {
                DWORD written{};
                if(!WriteFile(file,bytes.data()+offset,static_cast<DWORD>(bytes.size()-offset),&written,nullptr)||!written)
                    return false;
                offset+=written;
            }
            return true;
        } catch(...) { return false; }
    }
    void write_loop(const Json& manifest) noexcept {
        if(!write_json(manifest)) write_failed.store(true);
        for(;;) {
            const auto signal=wake.load(std::memory_order_acquire);
            const auto index=tail.load(std::memory_order_relaxed);
            if(index!=head.load(std::memory_order_acquire)) {
                if(!write_json(*queue[index])) write_failed.store(true);
                queue[index].reset();
                tail.store((index+1)%queue.size(),std::memory_order_release);
                continue;
            }
            if(stopping.load(std::memory_order_acquire)) break;
            wake.wait(signal,std::memory_order_acquire);
        }
        if(!FlushFileBuffers(file)) write_failed.store(true);
        try {
            if(!write_json({{"type","session_end"},{"normal_exit",normal_exit},{"accepted",accepted.load()},
                {"dropped",dropped.load()},{"write_failed",write_failed.load()},
                {"durability","final_flush_reported_by_recording_result"},
                {"complete",normal_exit&&!write_failed.load()&&dropped.load()==0}})) write_failed.store(true);
        } catch(...) { write_failed.store(true); }
        if(!FlushFileBuffers(file)) write_failed.store(true);
    }
    bool submit(Json event) {
        if(stopping.load(std::memory_order_acquire)) return false;
        const auto index=head.load(std::memory_order_relaxed),next=(index+1)%queue.size();
        if(next==tail.load(std::memory_order_acquire)||write_failed.load()) { ++dropped; return false; }
        queue[index]=std::move(event);
        ++accepted;
        head.store(next,std::memory_order_release);
        wake.fetch_add(1,std::memory_order_release); wake.notify_one();
        return true;
    }
    RecordingStats close(bool complete) {
        if(!closed) {
            normal_exit=complete;
            stopping.store(true,std::memory_order_release);
            wake.fetch_add(1,std::memory_order_release); wake.notify_one();
            if(writer.joinable()) writer.join();
            CloseHandle(file); file=INVALID_HANDLE_VALUE; closed=true;
        }
        return {accepted.load(),dropped.load(),write_failed.load(),normal_exit&&!write_failed.load()&&dropped.load()==0};
    }
    ~Impl() { close(false); }
};
SessionRecorder::SessionRecorder(const std::filesystem::path& path,Json manifest,std::size_t capacity):
    impl_(std::make_unique<Impl>(path,std::move(manifest),capacity)) {}
SessionRecorder::~SessionRecorder()=default;
bool SessionRecorder::submit(Json event) { return impl_->submit(std::move(event)); }
RecordingStats SessionRecorder::close(bool complete) { return impl_->close(complete); }
}
