#include "spectral_geometry.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace fantasynt {
namespace {
constexpr float pi = 3.14159265358979323846f;
constexpr int tube_sides = 6;
constexpr int ring_segments = 48;

struct Vec3 { float x, y, z; };
Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}
Vec3 normalized(Vec3 a) {
    return a * (1.0f / std::hypot(a.x, a.y, a.z));
}

void vertex(SurfaceGeometry& mesh, Vec3 p, Vec3 n, float u, float v) {
    mesh.vertices.push_back({p.x, p.y, p.z, n.x, n.y, n.z, u, v});
}
void triangle(SurfaceGeometry& mesh, std::size_t a, std::size_t b, std::size_t c) {
    mesh.indices.insert(mesh.indices.end(), {static_cast<std::uint16_t>(a),
        static_cast<std::uint16_t>(b), static_cast<std::uint16_t>(c)});
}

void ring(SurfaceGeometry& mesh, float height, float radius) {
    constexpr float thickness = 0.008f;
    // O primeiro anel toca o chão sem atravessar o plano y=0.
    const float center_y = std::max(height, thickness);
    const std::size_t first = mesh.vertices.size();
    for (int segment = 0; segment <= ring_segments; ++segment) {
        const float angle = 2 * pi * (segment % ring_segments) / ring_segments;
        const float ca = std::cos(angle), sa = std::sin(angle);
        for (int side = 0; side <= tube_sides; ++side) {
            const float section = pi / 2 + 2 * pi * (side % tube_sides) / tube_sides;
            const float cs = std::cos(section), ss = std::sin(section);
            const Vec3 position{(radius + thickness * cs) * ca,
                center_y + thickness * ss, (radius + thickness * cs) * sa};
            vertex(mesh, position, {cs * ca, ss, cs * sa},
                static_cast<float>(segment) / ring_segments, position.y / 2);
        }
    }
    for (int segment = 0; segment < ring_segments; ++segment) {
        for (int side = 0; side < tube_sides; ++side) {
            const auto a = first + segment * (tube_sides + 1) + side;
            const auto b = a + tube_sides + 1;
            triangle(mesh, a, a + 1, b);
            triangle(mesh, b, a + 1, b + 1);
        }
    }
}

void strut(SurfaceGeometry& mesh, Vec3 start, Vec3 end, float u) {
    constexpr float thickness = 0.004f;
    const Vec3 direction = normalized(end - start);
    const Vec3 tangent = normalized(cross(direction, {0, 1, 0}));
    const Vec3 bitangent = cross(direction, tangent);
    std::array<Vec3, tube_sides> rim_start{}, rim_end{};
    const auto first = mesh.vertices.size();
    for (int row = 0; row < 2; ++row) {
        for (int side = 0; side <= tube_sides; ++side) {
            const float angle = 2 * pi * (side % tube_sides) / tube_sides;
            const Vec3 normal = tangent * std::cos(angle) + bitangent * std::sin(angle);
            const Vec3 point = (row == 0 ? start : end) + normal * thickness;
            vertex(mesh, point, normal, u, point.y / 2);
            if (side < tube_sides) (row == 0 ? rim_start : rim_end)[side] = point;
        }
    }
    for (int side = 0; side < tube_sides; ++side) {
        const auto a = first + side, b = a + tube_sides + 1;
        triangle(mesh, a, a + 1, b);
        triangle(mesh, b, a + 1, b + 1);
    }
    // Tampas reais para que os tubos também sejam fechados nas extremidades.
    for (int cap = 0; cap < 2; ++cap) {
        const auto center_index = mesh.vertices.size();
        const Vec3 center = cap == 0 ? start : end;
        const Vec3 normal = direction * (cap == 0 ? -1.0f : 1.0f);
        vertex(mesh, center, normal, u, center.y / 2);
        for (const auto point : (cap == 0 ? rim_start : rim_end))
            vertex(mesh, point, normal, u, point.y / 2);
        for (int side = 0; side < tube_sides; ++side) {
            const auto a = center_index + 1 + side;
            const auto b = center_index + 1 + (side + 1) % tube_sides;
            if (cap == 0) triangle(mesh, center_index, b, a);
            else triangle(mesh, center_index, a, b);
        }
    }
}

template <std::size_t N>
void facet(SurfaceGeometry& mesh, const std::array<Vec3, N>& points,
           const std::array<std::array<float, 2>, N>& uv) {
    const Vec3 normal = normalized(cross(points[1] - points[0], points[2] - points[0]));
    Vec3 center{};
    float center_u = 0, center_v = 0;
    const auto first = mesh.vertices.size();
    for (std::size_t i = 0; i < N; ++i) {
        vertex(mesh, points[i], normal, uv[i][0], uv[i][1]);
        center = center + points[i] * (1.0f / N);
        center_u += uv[i][0] / N;
        center_v += uv[i][1] / N;
    }
    vertex(mesh, center, normal, center_u, center_v);
    for (std::size_t i = 0; i < N; ++i)
        triangle(mesh, first + N, first + i, first + (i + 1) % N);
}

float variation(int variant, int piece, std::uint32_t channel) {
    // Mistura inteira estável; a geometria não depende do relógio nem de estado global.
    std::uint32_t value = 0x9e3779b9u * (static_cast<std::uint32_t>(variant) + 1u)
        + 0x85ebca6bu * (static_cast<std::uint32_t>(piece) + 1u) + channel * 0xc2b2ae35u;
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    return static_cast<float>(value & 0xffffu) / 65535.0f;
}
}

SurfaceGeometry make_luminous_cone_piece_geometry(int piece) {
    SurfaceGeometry mesh;
    if (piece < 0 || piece >= spectral_cone_piece_count) return mesh;
    if (piece < spectral_cone_rib_count) {
        const float angle = 2 * pi * piece / spectral_cone_rib_count;
        constexpr float lower = 0.004f, upper = 1.996f;
        const Vec3 start{(1 - lower / 2) * std::cos(angle), lower,
            (1 - lower / 2) * std::sin(angle)};
        const Vec3 end{(1 - upper / 2) * std::cos(angle), upper,
            (1 - upper / 2) * std::sin(angle)};
        strut(mesh, start, end, static_cast<float>(piece) / spectral_cone_rib_count);
    } else {
        const float height = 2.0f * (piece - spectral_cone_rib_count) / spectral_cone_ring_count;
        ring(mesh, height, 1 - height / 2);
    }
    return mesh;
}

SurfaceGeometry make_crystal_piece_geometry(int variant, int piece) {
    SurfaceGeometry mesh;
    if (variant < 0 || variant >= crystal_variant_count ||
        piece < 0 || piece >= crystal_piece_count) return mesh;

    const float jitter = variation(variant, piece, 1);
    float angle = 2 * pi * piece / 8 + 0.37f * variant;
    float distance = piece == 0 ? 0.0f : 0.34f + 0.26f * jitter;
    switch (variant) {
    case 1: angle = -0.72f * pi + piece * 0.17f * pi; distance *= 1.22f; break;
    case 2: angle = (piece % 2 == 0 ? 0.0f : pi) + 0.55f * (jitter - 0.5f);
            distance = piece == 0 ? 0.12f : 0.20f + 0.52f * jitter; break;
    case 3: angle = 2.39996323f * piece; distance = 0.09f * piece; break;
    case 4: angle = 2 * pi * piece / 8 + 0.13f;
            distance = piece == 0 ? 0.0f : 0.22f + 0.22f * jitter; break;
    case 5: angle = pi * (piece % 3) / 1.5f + 0.8f * jitter;
            distance = piece == 0 ? 0.1f : 0.24f + 0.44f * jitter; break;
    default: break;
    }
    const float root_y = 0.10f * variation(variant, piece, 2);
    const float height = piece == 0 ? 1.86f + 0.07f * jitter :
        0.68f + 1.06f * variation(variant, piece, 3);
    const float width = (piece == 0 ? 0.21f : 0.12f) +
        0.07f * variation(variant, piece, 4);
    const float lean = piece == 0 ? 0.045f : 0.12f + 0.18f * variation(variant, piece, 5);
    const float lean_angle = angle + 0.50f * (variation(variant, piece, 6) - 0.5f);
    const Vec3 root{distance * std::cos(angle), root_y, distance * std::sin(angle)};
    const Vec3 axis{lean * std::cos(lean_angle), 1, lean * std::sin(lean_angle)};
    const Vec3 shoulder = root + axis * (height * 0.76f);
    const Vec3 tip = root + axis * height;
    const float twist = angle + 0.65f * variation(variant, piece, 7);
    const float shoulder_width = width * (0.72f + 0.18f * variation(variant, piece, 8));
    std::array<Vec3, 6> bottom{}, top{};
    for (int side = 0; side < 6; ++side) {
        const float a = twist + 2 * pi * side / 6;
        bottom[side] = root + Vec3{width * std::cos(a), 0, width * std::sin(a)};
        top[side] = shoulder + Vec3{shoulder_width * std::cos(a), 0,
            shoulder_width * std::sin(a)};
    }
    constexpr std::array<std::array<float, 2>, 4> side_uv{{{0, 0}, {0, 1}, {1, 1}, {1, 0}}};
    constexpr std::array<std::array<float, 2>, 3> tip_uv{{{0, 0}, {0.5f, 1}, {1, 0}}};
    for (int side = 0; side < 6; ++side) {
        const int next = (side + 1) % 6;
        facet(mesh, std::array<Vec3, 4>{bottom[side], top[side], top[next], bottom[next]}, side_uv);
        facet(mesh, std::array<Vec3, 3>{top[side], tip, top[next]}, tip_uv);
    }
    std::array<std::array<float, 2>, 6> base_uv{};
    for (int side = 0; side < 6; ++side) {
        const float a = twist + 2 * pi * side / 6;
        base_uv[side] = {0.5f + 0.5f * std::cos(a), 0.5f + 0.5f * std::sin(a)};
    }
    facet(mesh, bottom, base_uv);
    return mesh;
}
}
