#include "resourceshader.hpp"
#include "shader.hpp"
#include "global.hpp"
#include "HashedString.hpp"
#include "resourcefile.hpp"
#include "resourcemanager.hpp"
#include "resourcecache.hpp"

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
    ResourceCache* cache = Global::resourceManager()->Cache();
    assert(cache);
    const size_t string_length_max = 2048;
    char shader_path[string_length_max];
    snprintf(shader_path, string_length_max, "../shaders/%s.vert", name.c_str());
    mDependencies[0] = cache->get_or_create<ResourceFile>(std::string(shader_path));
    snprintf(shader_path, string_length_max, "../shaders/%s.frag", name.c_str());
    mDependencies[1] = cache->get_or_create<ResourceFile>(std::string(shader_path));
}

ResourceShader::~ResourceShader()
{
}

bool ResourceShader::Load()
{
    if(mShaderProgram)
        return true;
    bool result = true;
    for(auto& dependencies : mDependencies)
    {
        const bool loaded = dependencies->Load();
        result = result && loaded;
    }
    if (!result)
    {
        return result;
    }
    mShaderProgram = Global::resourceManager()->shader(name());
    if (!mShaderProgram)
    {
        result = false;
        return result;
    }
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
    return result;
}

void ResourceShader::PreloadAttribute(const HashedString& name)
{
    mParameters.push_back({ShaderParameterType::Attribute, name});
}

void ResourceShader::PreloadUniform(const HashedString& name)
{
    mParameters.push_back({ ShaderParameterType::Uniform, name });
}
