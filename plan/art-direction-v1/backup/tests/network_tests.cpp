#include <winsock2.h>
#include <ws2tcpip.h>
#include "net/control_server.hpp"
#include "net/line_framer.hpp"
#include "control/parameters.hpp"
#include "test_support.hpp"
#include <chrono>
#include <thread>
#include <stdexcept>

using namespace fantasynt;
using namespace std::chrono_literals;

void framing() {
    LineFramer framer;
    CHECK(framer.feed("{\"id\":").empty());
    auto frames=framer.feed("\"a\"}\r\n{\"id\":\"b\"}\n");
    CHECK(frames.size()==2);
    if(frames.size()==2) {
        CHECK(frames[0].payload.value()=="{\"id\":\"a\"}");
        CHECK(frames[1].payload.value()=="{\"id\":\"b\"}");
    }
    LineFramer limits;
    frames=limits.feed(std::string(max_message_bytes,'a')+"\r\n");
    CHECK(frames.size()==1);
    if(frames.size()==1) CHECK(frames[0].payload->size()==max_message_bytes);
    frames=limits.feed(std::string(max_message_bytes+1,'x'));
    CHECK(frames.size()==1);
    if(frames.size()==1) CHECK(frames[0].oversized && !frames[0].payload);
    frames=limits.feed("discard\nvalid\n");
    CHECK(frames.size()==1);
    if(frames.size()==1) CHECK(frames[0].payload.value()=="valid");
}
struct Client {
    SOCKET socket{INVALID_SOCKET};
    explicit Client(std::uint16_t port) {
        socket=::socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        sockaddr_in address{}; address.sin_family=AF_INET;
        address.sin_addr.s_addr=htonl(INADDR_LOOPBACK); address.sin_port=htons(port);
        if(connect(socket,reinterpret_cast<sockaddr*>(&address),sizeof(address))!=0)
            throw std::runtime_error("Client connection failed");
        DWORD timeout=2000;
        setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<const char*>(&timeout),sizeof(timeout));
    }
    ~Client() { if(socket!=INVALID_SOCKET) closesocket(socket); }
    void send_all(const std::string& message) {
        std::size_t pos=0;
        while(pos<message.size()) {
            int n=send(socket,message.data()+pos,static_cast<int>(message.size()-pos),0);
            if(n<=0) throw std::runtime_error("send failed");
            pos+=n;
        }
    }
};
std::vector<PendingMessage> wait_messages(ControlServer& server, std::size_t count) {
    std::vector<PendingMessage> result;
    for(int i=0;i<200 && result.size()<count;++i) {
        auto messages=server.drain();
        for(auto& m:messages) result.push_back(std::move(m));
        std::this_thread::sleep_for(5ms);
    }
    return result;
}
void sockets_and_shutdown() {
    const auto start=std::chrono::steady_clock::now();
    std::uint16_t port{};
    {
        ControlServer server(0); port=server.port(); CHECK(port!=0);
        Client client(port);
        client.send_all("one");
        std::this_thread::sleep_for(30ms);
        CHECK(server.drain().empty());
        client.send_all("\ntwo\nthree\r\n");
        auto messages=wait_messages(server,3);
        CHECK(messages.size()==3);
        if(messages.size()==3) {
            CHECK(messages[0].payload=="one"); CHECK(messages[1].payload=="two");
            CHECK(messages[2].payload=="three");
            server.reply(messages[0].client_id,R"({"status":"accepted"})");
            std::string response;
            while(response.find('\n')==std::string::npos) {
                char bytes[128]; int n=recv(client.socket,bytes,sizeof(bytes),0);
                if(n<=0) break;
                response.append(bytes,n);
            }
            CHECK(response=="{\"status\":\"accepted\"}\n");
        }
        bool occupied=false;
        try { ControlServer another(port); } catch(const std::exception&) { occupied=true; }
        CHECK(occupied);
        { Client incomplete(port); incomplete.send_all("unfinished"); }
        std::this_thread::sleep_for(50ms);
        CHECK(server.drain().empty());
    }
    CHECK(std::chrono::steady_clock::now()-start < 2s);
    { ControlServer rebound(port); CHECK(rebound.port()==port); }
    const auto stop=std::chrono::steady_clock::now();
    { ControlServer empty(0); }
    CHECK(std::chrono::steady_clock::now()-stop < 2s);
}
void saturation_and_oversize() {
    ControlServer server(0); Client client(server.port());
    std::string messages;
    for(int i=0;i<100;++i) messages+="x\n";
    client.send_all(messages);
    std::this_thread::sleep_for(150ms);
    auto input=server.drain(); CHECK(input.size()==64);
    char bytes[8192]; int n=recv(client.socket,bytes,sizeof(bytes),0);
    CHECK(n>0);
    if(n>0) CHECK(std::string(bytes,n).find("queue_full")!=std::string::npos);
    client.send_all(std::string(max_message_bytes+1,'x')+"\nok\n");
    input=wait_messages(server,1);
    CHECK(input.size()==1);
    if(input.size()==1) CHECK(input[0].payload=="ok");
}
int main() {
    framing();
    WSADATA data{}; WSAStartup(MAKEWORD(2,2),&data);
    try { sockets_and_shutdown(); saturation_and_oversize(); }
    catch(const std::exception& e) { CHECK(false); std::cerr<<e.what()<<'\n'; }
    WSACleanup();
    return test_finish();
}
