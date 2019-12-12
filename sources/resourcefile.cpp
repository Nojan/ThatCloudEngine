#include "resourcefile.hpp"
#include "global.hpp"
#include "platform/platform.hpp"

ResourceFile::ResourceFile(const std::string& name)
: Resource(name, ResourceType::File)
{

}

bool ResourceFile::Load()
{
    return Global::platform()->Fetch(name().c_str());
}
