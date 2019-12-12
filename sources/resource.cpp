#include "resource.hpp"
#include <cassert>

Resource::Resource(const std::string name, ResourceType type)
: mName(name)
, mType(type)
{}

bool Resource::Load()
{
    assert(false);
    return false;
}
