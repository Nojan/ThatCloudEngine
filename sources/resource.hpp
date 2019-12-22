#pragma once

#include <string>
#include <vector>

enum class ResourceType {
    Shader,
    File,
    Mesh,
    Invalid,
};

class Resource {
public:
    Resource() = delete;
    Resource(const std::string name, ResourceType type);
    virtual ~Resource() = default;

    virtual bool Load(Resource* owner = nullptr);
    virtual void OnDependencyLoad(const Resource* dependency);
    virtual void GetDependencies(std::vector<Resource*>& dependencies);

    const std::string& name() const { return mName; }
    ResourceType type() const { return mType; }
private:
    const std::string mName;
    const ResourceType mType;
protected:
    std::vector<Resource*> mOwners;
};
