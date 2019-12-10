#pragma once

#include "classcache.hpp"
#include "mesh_resource.hpp"

class MeshResourceCache : public ClassCache<MeshResourceList>
{
protected:
    std::shared_ptr<MeshResourceList> load(const std::string& name) const override;
};
