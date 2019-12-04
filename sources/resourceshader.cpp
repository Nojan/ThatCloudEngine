#include "resourceshader.hpp"
#include "shader.hpp"
#include "global.hpp"
#include "HashedString.hpp"
#include "resourcemanager.hpp"

enum class ShaderParameterType {
    Uniform,
    Attribute,
};

struct ResourceShader::ShaderParameter
{
    ShaderParameterType type;
    HashedString name;
};

ResourceShader::ResourceShader(const std::string name) 
: Resource(name, ResourceType::Shader) 
{
}

ResourceShader::~ResourceShader()
{
}

void ResourceShader::Load()
{
    if(mShaderProgram)
        return;
    mShaderProgram = Global::resourceManager()->shader(name());
    mShaderProgram->Bind();
    for (const auto& p: mParameters)
    {
        switch (p.type)
        {
            case ShaderParameterType::Attribute:
                mShaderProgram->RegisterAttrib(p.name);
                break;
            case ShaderParameterType::Uniform:
                mShaderProgram->RegisterUniform(p.name);
                break;
            default:
                assert(false);
        }
    }
    mShaderProgram->Unbind();
}

void ResourceShader::PreloadAttribute(const HashedString& name)
{
    mParameters.push_back({ShaderParameterType::Attribute, name});
}

void ResourceShader::PreloadUniform(const HashedString& name)
{
    mParameters.push_back({ ShaderParameterType::Uniform, name });
}
