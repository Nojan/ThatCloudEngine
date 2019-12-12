#include "resourcefile.hpp"
#include "global.hpp"
#include "platform/platform.hpp"

ResourceFile::ResourceFile(const std::string& name)
: Resource(name, ResourceType::File)
{

}

void ResourceFile::Load()
{
    Global::platform()->Fetch(name().c_str());
}
