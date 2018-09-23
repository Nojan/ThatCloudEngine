#include "mesh_resource_cache.hpp"
#include "resource_compiler_mesh.hpp"

std::shared_ptr<MeshResourceList> MeshResourceCache::load(const std::string & name) const
{
    std::shared_ptr<MeshResourceList> meshList = std::make_shared<MeshResourceList>();
    resource_compiler::compile_mesh(name.c_str(), *meshList);
    return meshList;
}
