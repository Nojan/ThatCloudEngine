#include "resourcecache.hpp"
#include "resource.hpp"

std::shared_ptr<Resource> ResourceCache::get_impl(const std::string &name)
{
    std::shared_ptr<Resource> resource;
    {
        auto it = mCache.find(name);
        if (mCache.end() != it)
        {
            std::weak_ptr<Resource> resource_obs = it->second;
            resource = resource_obs.lock();
            if (!resource)
            {
                mCache.erase(it);
            }
        }
    }
    return resource;
}

void ResourceCache::insert(std::shared_ptr<Resource> &resource)
{
    mCache[resource->name()] = std::weak_ptr<Resource>(resource);
}
