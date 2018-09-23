#pragma once

#include "ressourcecache.hpp"
#include "mesh_resource.hpp"

class MeshResourceCache : public RessourceCache<MeshResourceList>
{
protected:
    std::shared_ptr<MeshResourceList> load(const std::string& name) const override;
};
