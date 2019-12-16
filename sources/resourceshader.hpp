#pragma once
#include "resource.hpp"
#include <array>
#include <memory>
#include <vector>

class ResourceFile;
class ShaderProgram;
class HashedString;

class ResourceShader : public Resource {
public:
    ResourceShader(const std::string name);
    ~ResourceShader();

    bool Load(Resource* owner = nullptr) override;
    void OnDependencyLoad(const Resource* dependency) override;

    void PreloadAttribute(const HashedString& name);
    void PreloadUniform(const HashedString& name);

    std::shared_ptr<ShaderProgram> mShaderProgram;
private:
    struct ShaderParameter;
    std::vector<ShaderParameter> mParameters;
    std::array<std::shared_ptr<ResourceFile>, 2> mDependencies;
};
