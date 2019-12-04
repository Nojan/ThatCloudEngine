#include "resourceshader.hpp"
#include "shader.hpp"

ResourceShader::~ResourceShader()
{
}

ResourceShader::Load()
{
    mShaderProgram = Global::resourceManager()->shader(mName);
}
