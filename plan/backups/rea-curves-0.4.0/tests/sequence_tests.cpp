#include "render/sequence.hpp"
#include "test_support.hpp"
#include <cmath>
#include <limits>

using namespace fantasynt;
int main() {
    CHECK(sequence_duration==424);
    const auto initial=sample_sequence(0);
    CHECK(initial.weights[0]==1 && initial.weights[1]==0 && initial.weights[2]==0);
    CHECK(sample_sequence(79.99).weights[0]==1);
    CHECK(sample_sequence(96).weights[0]>0 && sample_sequence(96).weights[1]>0);
    CHECK(sample_sequence(112).weights[1]==1);
    CHECK(sample_sequence(223.99).weights[1]==1);
    CHECK(sample_sequence(228).weights[1]>0 && sample_sequence(228).weights[2]>0);
    CHECK(sample_sequence(232).weights[2]==1);
    CHECK(sample_sequence(300).weights[2]==1);
    CHECK(sample_sequence(424).exposure==0);
    CHECK(sample_sequence(-20).weights==initial.weights);
    CHECK(sample_sequence(std::numeric_limits<double>::quiet_NaN()).weights==initial.weights);
    for(double t=0;t<=424;t+=0.1) {
        const auto a=sample_sequence(t),b=sample_sequence(t+0.001);
        float sum{};
        for(std::size_t i=0;i<3;++i) {
            CHECK(a.weights[i]>=0 && a.weights[i]<=1);
            CHECK(std::abs(a.weights[i]-b.weights[i])<0.002);
            sum+=a.weights[i];
        }
        CHECK(std::abs(sum-1)<0.0001);
        CHECK(a.exposure>=0 && a.exposure<=1);
        CHECK(std::isfinite(a.atmosphere.r+a.atmosphere.g+a.atmosphere.b));
    }
    CHECK(sample_sequence(207).atmosphere.r>sample_sequence(144).atmosphere.r);
    return test_finish();
}
