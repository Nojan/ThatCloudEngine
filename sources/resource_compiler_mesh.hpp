#pragma once

#include "mesh_resource.hpp"
class Mesh;

namespace resource_compiler {
    void compile_mesh(const char* filepath, MeshResourceList& meshList);
}
