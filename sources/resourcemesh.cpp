#include "resourcemesh.hpp"
#include "global.hpp"
#include "resourcefile.hpp"
#include "resourcemanager.hpp"
#include "resourcecache.hpp"
#include "resource_compiler_mesh.hpp"

#include <cassert>

ResourceMesh::ResourceMesh(const std::string name) 
: Resource(name, ResourceType::Mesh) 
{
    ResourceCache* cache = Global::resourceManager()->Cache();
    assert(cache);
    const size_t string_length_max = 2048;
    char mesh_path[string_length_max];
    snprintf(mesh_path, string_length_max, "../assets/3D/%s.assxml", name.c_str());
    mDependencies.push_back(cache->get_or_create<ResourceFile>(std::string(mesh_path)));
}

ResourceMesh::~ResourceMesh()
{
}

bool ResourceMesh::Load(Resource* owner)
{
    if(mMesh)
        return true;
    bool result = true;
    for (auto& d : mDependencies)
    {
        const bool loaded = d->Load(this);
        result = result && loaded;
    }
    if (!result)
        return result;
    mMesh = Global::resourceManager()->meshResource(mDependencies[0]->name());
    return result;
}

void ResourceMesh::OnDependencyLoad(const Resource* dependency)
{
    if (dependency == mDependencies[0].get())
    {
        resource_compiler::get_dependencies(*mDependencies[0], mDependencies);
    }
    Load(nullptr);
}

void ResourceMesh::GetDependencies(std::vector<Resource*>& dependencies)
{
    dependencies.reserve(dependencies.size() + mDependencies.size());
    for (auto& dependency : mDependencies)
    {
        dependencies.push_back(dependency.get());
    }
}

std::shared_ptr<MeshResourceList> ResourceMesh::Mesh()
{
    return mMesh;
}
