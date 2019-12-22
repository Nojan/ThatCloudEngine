#include "resource.hpp"
#include <cassert>

Resource::Resource(const std::string name, ResourceType type)
: mName(name)
, mType(type)
{}

bool Resource::Load(Resource* owner)
{
    assert(false);
    return false;
}

void Resource::OnDependencyLoad(const Resource* dependency)
{
    assert(false);
}

void Resource::GetDependencies(std::vector<Resource*>& dependencies)
{
}
