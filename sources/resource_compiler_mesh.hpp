#pragma once

#include "mesh_resource.hpp"
#include <vector>
#include <memory>

class Mesh;
class ResourceFile;

namespace resource_compiler {
    void compile_mesh(const char* filepath, MeshResourceList& meshList);
    void get_dependencies(const ResourceFile& meshfile, std::vector<std::shared_ptr<ResourceFile>>& dependencies);
}
