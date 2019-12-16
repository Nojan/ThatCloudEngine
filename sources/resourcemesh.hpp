#pragma once
#include "resource.hpp"
#include "mesh_resource.hpp"
#include <memory>
#include <vector>

class ResourceFile;

class ResourceMesh : public Resource {
public:
    ResourceMesh(const std::string name);
    ~ResourceMesh();

    bool Load(Resource* owner = nullptr) override;
    void OnDependencyLoad(const Resource* dependency) override;

    std::shared_ptr<MeshResourceList> Mesh();
private:
    std::vector<std::shared_ptr<ResourceFile>> mDependencies;
    std::shared_ptr<MeshResourceList> mMesh;
};
