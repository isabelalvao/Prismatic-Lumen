#include "render/spectral_geometry.hpp"
#include "test_support.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <map>
#include <utility>

using namespace fantasynt;

namespace {
void check_mesh(const SurfaceGeometry& mesh) {
    CHECK(!mesh.vertices.empty() && mesh.vertices.size() < 65536);
    CHECK(!mesh.indices.empty() && mesh.indices.size() % 3 == 0);
    for (const auto& vertex : mesh.vertices) {
        for (float value : {vertex.x, vertex.y, vertex.z, vertex.nx,
                            vertex.ny, vertex.nz, vertex.u, vertex.v})
            CHECK(std::isfinite(value));
        CHECK_NEAR(std::hypot(vertex.nx, vertex.ny, vertex.nz), 1.0f, 0.001f);
        CHECK(vertex.u >= 0 && vertex.u <= 1);
        CHECK(vertex.v >= 0 && vertex.v <= 1);
    }
    for (auto index : mesh.indices) CHECK(index < mesh.vertices.size());
    for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const auto& a = mesh.vertices[mesh.indices[i]];
        const auto& b = mesh.vertices[mesh.indices[i + 1]];
        const auto& c = mesh.vertices[mesh.indices[i + 2]];
        const float ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
        const float vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
        const float nx = uy * vz - uz * vy;
        const float ny = uz * vx - ux * vz;
        const float nz = ux * vy - uy * vx;
        CHECK(std::hypot(nx, ny, nz) > 1e-9f);
        // A orientação exterior concorda com as normais da superfície.
        CHECK(nx * (a.nx + b.nx + c.nx) +
              ny * (a.ny + b.ny + c.ny) +
              nz * (a.nz + b.nz + c.nz) > 0);
    }
}

void check_closed_surface(const SurfaceGeometry& mesh) {
    using Position = std::array<long, 3>;
    using Edge = std::pair<Position, Position>;
    std::map<Edge, int> uses;
    auto position = [&](std::uint16_t index) {
        const auto& v = mesh.vertices[index];
        return Position{std::lround(v.x * 1000000), std::lround(v.y * 1000000),
                        std::lround(v.z * 1000000)};
    };
    // As costuras de UV e as normais de faces duplicam vértices na mesma posição.
    for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
        for (int edge = 0; edge < 3; ++edge) {
            Position a = position(mesh.indices[i + edge]);
            Position b = position(mesh.indices[i + (edge + 1) % 3]);
            if (b < a) std::swap(a, b);
            ++uses[{a, b}];
        }
    }
    for (const auto& entry : uses) CHECK(entry.second == 2);
}

void check_flat_facets(const SurfaceGeometry& mesh) {
    for (std::size_t i = 0; i < mesh.indices.size(); i += 3) {
        const auto& a = mesh.vertices[mesh.indices[i]];
        for (int corner = 1; corner < 3; ++corner) {
            const auto& b = mesh.vertices[mesh.indices[i + corner]];
            CHECK_NEAR(a.nx, b.nx, 1e-6f);
            CHECK_NEAR(a.ny, b.ny, 1e-6f);
            CHECK_NEAR(a.nz, b.nz, 1e-6f);
            CHECK_NEAR(a.nx * (b.x - a.x) + a.ny * (b.y - a.y) +
                       a.nz * (b.z - a.z), 0, 1e-6f);
        }
    }
}

bool same_mesh(const SurfaceGeometry& a, const SurfaceGeometry& b) {
    if (a.indices != b.indices || a.vertices.size() != b.vertices.size()) return false;
    for (std::size_t i = 0; i < a.vertices.size(); ++i) {
        const auto& av = a.vertices[i];
        const auto& bv = b.vertices[i];
        if (av.x != bv.x || av.y != bv.y || av.z != bv.z ||
            av.nx != bv.nx || av.ny != bv.ny || av.nz != bv.nz ||
            av.u != bv.u || av.v != bv.v) return false;
    }
    return true;
}
}

int main() {
    // Regressão: cada nervura deve ser uma peça completa, separada dos círculos.
    CHECK(spectral_cone_piece_count == 17);
    float previous_radius = 2, overall_top = 0;
    for (int piece = 0; piece < spectral_cone_piece_count; ++piece) {
        const auto mesh = make_luminous_cone_piece_geometry(piece);
        check_mesh(mesh);
        check_closed_surface(mesh);
        CHECK(same_mesh(mesh, make_luminous_cone_piece_geometry(piece)));
        float widest = 0, lowest = 2, highest = 0;
        for (const auto& vertex : mesh.vertices) {
            const float radius = std::hypot(vertex.x, vertex.z);
            CHECK(vertex.y >= -1e-6f && vertex.y <= 2.000001f);
            CHECK(radius <= 1.04f);
            CHECK(std::abs(radius - (1 - vertex.y / 2)) < 0.020f);
            CHECK_NEAR(vertex.v, vertex.y / 2, 1e-6f);
            widest = std::max(widest, radius);
            lowest = std::min(lowest, vertex.y);
            overall_top = std::max(overall_top, vertex.y);
            highest = std::max(highest, vertex.y);
        }
        if (piece < 8) {
            CHECK(lowest < 0.009f && highest > 1.99f);
        } else {
            CHECK(highest - lowest < 0.017f);
            CHECK(widest < previous_radius);
            CHECK_NEAR(lowest, 2.0f * (piece - 8) / 9, 0.009f);
            previous_radius = widest;
        }
    }
    CHECK(overall_top > 1.995f);
    CHECK(make_luminous_cone_piece_geometry(-1).vertices.empty());
    CHECK(make_luminous_cone_piece_geometry(17).vertices.empty());

    std::array<SurfaceGeometry, crystal_variant_count> representatives;
    for (int variant = 0; variant < crystal_variant_count; ++variant) {
        std::array<SurfaceGeometry, crystal_piece_count> pieces;
        float tallest = 0;
        for (int piece = 0; piece < crystal_piece_count; ++piece) {
            pieces[piece] = make_crystal_piece_geometry(variant, piece);
            const auto& mesh = pieces[piece];
            check_mesh(mesh);
            check_closed_surface(mesh);
            check_flat_facets(mesh);
            CHECK(same_mesh(mesh, make_crystal_piece_geometry(variant, piece)));
            float lowest = 2;
            bool interior_uv = false;
            for (const auto& vertex : mesh.vertices) {
                CHECK(vertex.y >= 0 && vertex.y <= 2.000001f);
                CHECK(std::hypot(vertex.x, vertex.z) <= 1.8f);
                lowest = std::min(lowest, vertex.y);
                tallest = std::max(tallest, vertex.y);
                interior_uv |= vertex.u > 0 && vertex.u < 1 &&
                               vertex.v > 0 && vertex.v < 1;
            }
            CHECK(lowest <= 0.12f);
            CHECK(interior_uv);
            // Cada peça tem uma posição e um perfil próprios no aglomerado.
            for (int earlier = 0; earlier < piece; ++earlier)
                CHECK(!same_mesh(mesh, pieces[earlier]));
        }
        CHECK(tallest > 0.6f && tallest < 0.85f);
        representatives[variant] = pieces[0];
        for (int earlier = 0; earlier < variant; ++earlier)
            CHECK(!same_mesh(representatives[variant], representatives[earlier]));
    }
    CHECK(make_crystal_piece_geometry(-1, 0).vertices.empty());
    CHECK(make_crystal_piece_geometry(6, 0).vertices.empty());
    CHECK(make_crystal_piece_geometry(0, -1).vertices.empty());
    CHECK(make_crystal_piece_geometry(0, 9).vertices.empty());
    return test_finish();
}
