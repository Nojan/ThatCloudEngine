#pragma once

#include <string>

enum class ResourceType {
    Shader,
    File,
    Invalid,
};

class Resource {
public:
    Resource() = delete;
    Resource(const std::string name, ResourceType type);
    virtual ~Resource() = default;

    virtual void Load() {}

    const std::string& name() const { return mName; }
    ResourceType type() const { return mType; }
private:
    const std::string mName;
    const ResourceType mType;
};
