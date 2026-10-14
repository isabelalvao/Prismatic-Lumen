#include "render/surface_choreography.hpp"
#include "test_support.hpp"

using namespace fantasynt;

int main() {
    SurfaceChoreographySettings settings;
    // Cada anel e as suas duas tampas pertencem ao mesmo segmento.
    for (int segment = 0; segment < 7; ++segment) {
        CHECK(cone_segment_index(segment * 2.0 / 7.0) == segment);
        CHECK(cone_segment_index((segment + 0.73) * 2.0 / 7.0) == segment);
    }
    CHECK(cone_segment_index(-1) == 0);
    CHECK(cone_segment_index(2) == 6);
    CHECK_NEAR(cone_segment_visibility(0, 0, 135, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 0, 136.4, settings), 0.5, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 0, 136.8, settings), 1, 1e-9);
    CHECK_NEAR(cone_segment_visibility(1, 0, 137.9, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_visibility(6, 1, 150.8, settings), 1, 1e-9);
    CHECK_NEAR(cone_segment_visibility(0, 1, 136.8, settings), 0, 1e-9);

    settings.cone_rotation_start_clock = 40;
    CHECK_NEAR(cone_segment_angle(0, 0, 40, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(0, 0, 40.8, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(0, 0, 41.8, settings), 0.034906585, 1e-9);
    CHECK_NEAR(cone_segment_angle(1, 0, 43.8, settings), -0.034906585, 1e-9);
    CHECK_NEAR(cone_segment_angle(2, 1, 45, settings), 0, 1e-9);
    CHECK_NEAR(cone_segment_angle(2, 1, 47.8, settings), 0.034906585, 1e-9);
    // A rotação usa exclusivamente o relógio de animação pausável.
    CHECK_NEAR(cone_segment_angle(6, 0, 52, settings), 0, 1e-9);

    CHECK_NEAR(previous_piece_visibility(0, 0), 1, 1e-9);
    CHECK_NEAR(previous_piece_visibility(0, 0.35), 0, 1e-9);
    CHECK_NEAR(previous_piece_visibility(1, 0.65), 1, 1e-9);
    CHECK_NEAR(previous_piece_visibility(1, 1), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.4, 0.99, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224, 1, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.4, 1, settings), 0.5, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224.8, 1, settings), 1, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0.5, 239.5, 1, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(1, 272, 1, settings), 1, 1e-9);
    CHECK_NEAR(generic_piece_visibility(1, 172, 172, settings), 0, 1e-9);
    CHECK_NEAR(generic_piece_visibility(0, 172.4, 172, settings), 0.5, 1e-9);
    CHECK_NEAR(generic_piece_visibility(1, 180, 172, settings), 1, 1e-9);
    settings.segment_reveal_seconds = 0;
    settings.tube_piece_reveal_seconds = 0;
    CHECK_NEAR(cone_segment_visibility(0, 0, 136, settings), 0, 1e-9);
    CHECK_NEAR(tube_piece_visibility(0, 224, 1, settings), 0, 1e-9);
    CHECK_NEAR(generic_piece_visibility(0, 172, 172, settings), 0, 1e-9);
    return test_finish();
}
