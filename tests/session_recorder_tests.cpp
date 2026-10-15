#include "control/session_recorder.hpp"
#include "test_support.hpp"
#include <filesystem>
#include <fstream>
#include <vector>

using namespace fantasynt;
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    const auto folder=std::filesystem::path(argv[1]);
    std::filesystem::create_directories(folder);
    const auto data=folder/(new_session_id()+".jsonl");
    SessionRecorder recorder(data,{{"session_id","test"}},64);
    CHECK(recorder.submit({{"type","observation"},{"seq",1}}));
    for(int i=2;i<=500;++i) {
        // A capacidade é menor que a rajada; preservar a ordem de tudo o que entra.
        recorder.submit({{"type","observation"},{"seq",i}});
    }
    auto result=recorder.close(true);
    CHECK(result.complete==(result.dropped==0));
    CHECK(result.accepted+result.dropped==500);
    std::ifstream input(data);
    std::string line; std::vector<nlohmann::json> rows;
    while(std::getline(input,line)) rows.push_back(nlohmann::json::parse(line));
    CHECK(rows.size()==result.accepted+2);
    CHECK(rows[0]["type"]=="session_manifest");
    CHECK(rows[1]["seq"]==1);
    CHECK(rows.back()["type"]=="session_end");
    CHECK(rows.back()["complete"]==result.complete);
    int previous=0;
    for(std::size_t i=1;i+1<rows.size();++i) {
        const int next=rows[i]["seq"];
        CHECK(next>previous); previous=next;
    }
    bool refused=false;
    try { SessionRecorder overwrite(data,{}); } catch(const std::exception&) { refused=true; }
    CHECK(refused);
    CHECK(!recorder.submit({}));
    input.close();
    std::filesystem::remove(data);

    const auto overflow=folder/(new_session_id()+".jsonl");
    SessionRecorder saturated(overflow,{},0);
    CHECK(!saturated.submit({{"type","observation"}}));
    auto incomplete=saturated.close(true);
    CHECK(!incomplete.complete); CHECK(incomplete.dropped==1);
    std::filesystem::remove(overflow);

    const auto aborted=folder/(new_session_id()+".jsonl");
    { SessionRecorder abandoned(aborted,{}); }
    std::ifstream abandoned(aborted); rows.clear();
    while(std::getline(abandoned,line)) rows.push_back(nlohmann::json::parse(line));
    CHECK(rows.back()["normal_exit"]==false);
    CHECK(rows.back()["complete"]==false);
    abandoned.close(); std::filesystem::remove(aborted);

    const auto asset=folder/(new_session_id()+".txt");
    { std::ofstream out(asset,std::ios::binary); out<<"abc"; }
    CHECK(file_sha256(asset)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    std::filesystem::remove(asset);
    return test_finish();
}
