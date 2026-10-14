#pragma once
#include <bgfx/bgfx.h>
namespace fantasynt {
struct Vertex { float x,y,z,nx,ny,nz; };
struct Mesh {
    bgfx::VertexBufferHandle vertices{BGFX_INVALID_HANDLE};
    bgfx::IndexBufferHandle indices{BGFX_INVALID_HANDLE};
    void destroy();
};
bgfx::VertexLayout vertex_layout();
Mesh make_box();
Mesh make_ground();
Mesh make_quad();
}
