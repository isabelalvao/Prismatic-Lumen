#include "render/sequence.hpp"
#include "test_support.hpp"
#include <cmath>
#include <limits>

using namespace fantasynt;
int main() {
    CHECK(sequence_duration==424);
    const auto initial=sample_sequence(0);
    CHECK((initial.opening_weight==1 && initial.weights==std::array<float,3>{}));
    CHECK(sample_sequence(79.99).opening_weight==1);
    CHECK(sample_sequence(96).opening_weight==1);
    CHECK(sample_sequence(112).opening_weight==1);
    CHECK(sample_sequence(124).opening_weight>0 && sample_sequence(124).weights[1]>0);
    CHECK(sample_sequence(223.99).weights[1]==1);
    CHECK(sample_sequence(228).weights[1]>0 && sample_sequence(228).weights[2]>0);
    CHECK(sample_sequence(232).weights[2]==1);
    CHECK(sample_sequence(300).weights[2]==1);
    CHECK(sample_sequence(424).exposure==0);
    CHECK(sample_sequence(-20).weights==initial.weights);
    CHECK(sample_sequence(std::numeric_limits<double>::quiet_NaN()).weights==initial.weights);
    for(double t=0;t<=424;t+=0.1) {
        const auto a=sample_sequence(t),b=sample_sequence(t+0.001);
        float sum=a.opening_weight;
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
    // Pontos interiores distinguem Linear/Constant da antiga suavização uniforme.
    CHECK_NEAR(sample_sequence(88).atmosphere.r,0.006f,0.000001f);
    CHECK_NEAR(sample_sequence(120).atmosphere.r,0.024f,0.000001f);
    CHECK_NEAR(sample_sequence(143.99).atmosphere.g,0.038f,0.000001f);
    CHECK_NEAR(sample_sequence(144).atmosphere.g,0.122f,0.000001f);
    CHECK_NEAR(sample_sequence(424).atmosphere.b,0.02f,0.000001f);
    CHECK_NEAR(sample_sequence(207.5).atmosphere.r,0.4942137f,0.000001f);
    const std::array<CurveKey,3> mixed{{{0,2,0,0,1},{2,8,0,0,0},{4,12,0,0,0}}};
    CHECK(evaluate_curve(mixed,1)==2);
    CHECK(evaluate_curve(mixed,2)==8);
    CHECK(evaluate_curve(mixed,3)==10);
    CHECK(evaluate_curve(mixed,-2)==2 && evaluate_curve(mixed,5)==12);
    CHECK(evaluate_curve({},2,7)==7);
    // Hermite em t=.25: .15625*2 + .140625*8 - .046875*(-4) = 1.625.
    const std::array<CurveKey,2> cubic{{{0,0,0,4,2},{2,2,-2,0,0}}};
    CHECK_NEAR(evaluate_curve(cubic,0.5f),1.625f,0.00001f);
    CHECK_NEAR(sample_camera_position(0),-6400.00048828125,0.001);
    CHECK_NEAR(sample_camera_position(168),4799.999755859375,0.002);
    CHECK_NEAR(sample_camera_position(224),16000,0.001);
    CHECK_NEAR(sample_camera_position(424),62400,0.001);
    CHECK_NEAR(sequence_camera_step(224,225,0.3f),200,0.002);
    CHECK(sequence_camera_step(224,225,0)==0);
    CHECK_NEAR(sequence_camera_step(224,225,0.6f),400,0.002);
    CHECK_NEAR(sequence_camera_step(220,230,0.3f),2000,0.002);
    return test_finish();
}
