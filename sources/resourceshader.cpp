#include "resourceshader.hpp"
#include "shader.hpp"
#include "global.hpp"
#include "resourcemanager.hpp"

ResourceShader::~ResourceShader()
{
}

void ResourceShader::Load()
{
    mShaderProgram = Global::resourceManager()->shader(name());
}
