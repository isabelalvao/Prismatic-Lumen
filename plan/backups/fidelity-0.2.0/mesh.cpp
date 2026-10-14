#include "mesh.hpp"
#include <array>
#include <stdexcept>

namespace fantasynt {
bgfx::VertexLayout vertex_layout() {
    bgfx::VertexLayout layout;
    layout.begin().add(bgfx::Attrib::Position,3,bgfx::AttribType::Float)
        .add(bgfx::Attrib::Normal,3,bgfx::AttribType::Float).end();
    return layout;
}
void Mesh::destroy() {
    if(bgfx::isValid(vertices)) bgfx::destroy(vertices);
    if(bgfx::isValid(indices)) bgfx::destroy(indices);
    vertices=BGFX_INVALID_HANDLE; indices=BGFX_INVALID_HANDLE;
}
template<std::size_t V,std::size_t I>
Mesh make_mesh(const std::array<Vertex,V>& vertices,const std::array<std::uint16_t,I>& indices) {
    Mesh mesh;
    mesh.vertices=bgfx::createVertexBuffer(bgfx::copy(vertices.data(),sizeof(vertices)),vertex_layout());
    mesh.indices=bgfx::createIndexBuffer(bgfx::copy(indices.data(),sizeof(indices)));
    if(!bgfx::isValid(mesh.vertices)||!bgfx::isValid(mesh.indices)) {
        mesh.destroy(); throw std::runtime_error("Cannot create mesh buffers");
    }
    return mesh;
}
Mesh make_box() {
    const std::array<Vertex,24> v{{
        {-1,-1,-1,0,0,-1},{1,-1,-1,0,0,-1},{1,1,-1,0,0,-1},{-1,1,-1,0,0,-1},
        {1,-1,1,0,0,1},{-1,-1,1,0,0,1},{-1,1,1,0,0,1},{1,1,1,0,0,1},
        {-1,-1,1,-1,0,0},{-1,-1,-1,-1,0,0},{-1,1,-1,-1,0,0},{-1,1,1,-1,0,0},
        {1,-1,-1,1,0,0},{1,-1,1,1,0,0},{1,1,1,1,0,0},{1,1,-1,1,0,0},
        {-1,1,-1,0,1,0},{1,1,-1,0,1,0},{1,1,1,0,1,0},{-1,1,1,0,1,0},
        {-1,-1,1,0,-1,0},{1,-1,1,0,-1,0},{1,-1,-1,0,-1,0},{-1,-1,-1,0,-1,0}
    }};
    std::array<std::uint16_t,36> indices{};
    for(std::uint16_t face=0;face<6;++face) {
        const auto b=static_cast<std::uint16_t>(face*4);
        const std::array<std::uint16_t,6> values{b,static_cast<std::uint16_t>(b+1),static_cast<std::uint16_t>(b+2),
            static_cast<std::uint16_t>(b+2),static_cast<std::uint16_t>(b+3),b};
        for(std::size_t i=0;i<6;++i) indices[face*6+i]=values[i];
    }
    return make_mesh(v,indices);
}
Mesh make_ground() {
    const std::array<Vertex,4> v{{{-1,0,-1,0,1,0},{1,0,-1,0,1,0},{1,0,1,0,1,0},{-1,0,1,0,1,0}}};
    return make_mesh(v,std::array<std::uint16_t,6>{0,1,2,2,3,0});
}
Mesh make_quad() {
    const std::array<Vertex,3> v{{{-1,-1,0,0,0,0},{3,-1,0,0,0,0},{-1,3,0,0,0,0}}};
    return make_mesh(v,std::array<std::uint16_t,3>{0,1,2});
}
}
