#pragma once
#include "resource.hpp"
#include <memory>
#include <vector>

class ShaderProgram;
class HashedString;

class ResourceShader : public Resource {
public:
    ResourceShader(const std::string name);
    ~ResourceShader();

    void Load() override;

    void PreloadAttribute(const HashedString& name);
    void PreloadUniform(const HashedString& name);

    std::shared_ptr<ShaderProgram> mShaderProgram;
private:
    struct ShaderParameter;
    std::vector<ShaderParameter> mParameters;
};
