#pragma once

#include <string>

enum class ResourceType {
    Shader,
    Invalid,
};

class Resource {
public:
    Resource() = delete;
    Resource(const std::string name, ResourceType type) : mName(name), mType(type) {}

    virtual void Load() {}

    const std::string& name() const { return mName; }
    const ResourceType type() const { return mType; }
private:
    const std::string mName;
    const ResourceType mType;
};
