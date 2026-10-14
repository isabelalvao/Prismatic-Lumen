#include "render/opening_scene.hpp"
#include "test_support.hpp"
#include <fstream>
#include <limits>

using namespace fantasynt;
int main(int argc,char** argv) {
    if(argc!=2) return 2;
    CHECK(opening_visible(OpeningStage::Intro,111.99));
    CHECK(!opening_visible(OpeningStage::Intro,112));
    CHECK(opening_visible(OpeningStage::Dome,79.99));
    CHECK(!opening_visible(OpeningStage::Dome,80));
    CHECK(!opening_visible(OpeningStage::Forest,79.99));
    CHECK(opening_visible(OpeningStage::Forest,80));
    CHECK(opening_visible(OpeningStage::Forest,120));
    CHECK(!opening_visible(OpeningStage::Forest,std::numeric_limits<double>::quiet_NaN()));
    const auto scene=load_opening_scene(std::filesystem::u8path(argv[1]));
    CHECK(!scene.groups.empty() && !scene.meshes.empty());
    std::size_t intro{},forest{};
    for(const auto& g:scene.groups) {
        CHECK(!g.instances.empty() && scene.meshes.contains(g.mesh));
        if(g.stage==OpeningStage::Forest) forest+=g.instances.size();
        else intro+=g.instances.size();
        for(const auto& i:g.instances) {
            CHECK(i.transform[15]==1);
            for(auto f:i.transform) CHECK(std::isfinite(f));
        }
    }
    CHECK(intro>100 && forest>1000);
    for(const auto& [id,m]:scene.meshes) {
        CHECK(!m.vertices.empty() && !m.indices.empty());
        CHECK(m.indices.size()%3==0);
        for(auto index:m.indices) CHECK(index<m.vertices.size());
    }
    // Um cabeçalho inválido não pode ser interpretado como contagens/ponteiros.
    const auto bad=std::filesystem::temp_directory_path()/"fantasynt-opening-invalid.fmesh";
    { std::ofstream output(bad,std::ios::binary); output<<"FNTMESH1"; }
    bool rejected=false;
    try { (void)load_reference_mesh(bad); } catch(const std::exception&) { rejected=true; }
    std::filesystem::remove(bad);
    CHECK(rejected);
    return test_finish();
}
