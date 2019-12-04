#pragma once
#include "resource.hpp"
#include <memory>

class ShaderProgram;

class ResourceShader : public Resource {
public:
    ResourceShader(const std::string name) : Resource(name, ResourceType::Shader) {}
    ~ResourceShader();

    void Load() override;

    std::shared_ptr<ShaderProgram> mShaderProgram;
};
