#pragma once
#include <bgfx/bgfx.h>
#include "geometry.hpp"
namespace fantasynt {
using Vertex=SurfaceVertex;
struct Mesh {
    bgfx::VertexBufferHandle vertices{BGFX_INVALID_HANDLE};
    bgfx::IndexBufferHandle indices{BGFX_INVALID_HANDLE};
    void destroy();
};
bgfx::VertexLayout vertex_layout();
Mesh make_box();
Mesh make_ground();
Mesh make_quad();
Mesh make_surface_mesh(const SurfaceGeometry& geometry);
}
