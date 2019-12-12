#pragma once
#include "resource.hpp"

class ResourceFile : public Resource
{
public:
    ResourceFile(const std::string& name);

    void Load() override;
private:
};
