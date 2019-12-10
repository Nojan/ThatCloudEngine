#pragma once

#include <string>
#include <unordered_map>
#include <memory>

class Resource;

class ResourceCache {
public:
    template<class T>
    std::shared_ptr<T> get(const std::string& name);
	
private:
    std::shared_ptr<Resource> get_impl(const std::string& name);
    void insert(std::shared_ptr<Resource>& resource);
    std::unordered_map<std::string, std::weak_ptr<Resource>> mCache;
};

template<class T>
std::shared_ptr<T> ResourceCache::get(const std::string& name) {
    std::shared_ptr<T> resource;
    std::shared_ptr<Resource> resource_base = get_impl(name);
    if(!resource_base)
    {
        resource = std::shared_ptr<T>(new T(name));
        resource_base = resource;
        insert(resource_base);
    }
    else
    {
        resource = std::dynamic_pointer_cast<T>(resource_base);
    }
    return resource;
}
