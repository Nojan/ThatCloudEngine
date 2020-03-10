#include "sdf_renderer.hpp"

#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "shader.hpp"
#include "resourceshader.hpp"
#include "root.hpp"

#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cassert>

float sdSphere(glm::vec3 p, float r)
{
    return glm::length(p) - r;
}

float sdBox(glm::vec3 p, glm::vec3 b)
{
    glm::vec3 d = glm::abs(p) - b;
    return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.f) + glm::length(glm::max(d, 0.f));
}

namespace Constant {
    IMGUI_VAR(Rotation, 0.f);
}


#if GUI_DEBUG()
void SDFRenderer::debug_GUI() const {
    ImGui::SliderFloat("Rotation", &Constant::Rotation, 0.f, 1.f);
}
#endif

SDFRenderer::SDFRenderer()
{
    mShaderResource = std::make_unique<ResourceShader>("sdf");
    mShaderResource->PreloadAttribute(HashedString("vPosition"));
    mShaderResource->PreloadUniform(HashedString("ro"));
    mShaderResource->PreloadUniform(HashedString("camera"));
    mShaderResource->PreloadUniform(HashedString("iResolution"));
    mShaderResource->PreloadUniform(HashedString("iVoxelDataCenter"));
    mShaderResource->PreloadUniform(HashedString("iVoxelDataSize"));
    mShaderResource->PreloadUniform(HashedString("iVoxelDataSize_rcp"));
    FlushFrame();

    mVertices.mElements.resize(4);
    mIndex.mElements.resize(6);

    constexpr float boundaryf = 1.0f;
    mVertices.mElements[0] = glm::vec2(-boundaryf, boundaryf);
    mVertices.mElements[1] = glm::vec2( boundaryf, boundaryf);
    mVertices.mElements[2] = glm::vec2( boundaryf,-boundaryf);
    mVertices.mElements[3] = glm::vec2(-boundaryf,-boundaryf);

    mIndex.mElements[0] = 0;
    mIndex.mElements[1] = 1;
    mIndex.mElements[2] = 3;

    mIndex.mElements[3] = 1;
    mIndex.mElements[4] = 2;
    mIndex.mElements[5] = 3;

    mVertices.StreamGPU();
    mIndex.StreamGPU();
}

SDFRenderer::~SDFRenderer()
{
}

const glm::vec3 g_VoxelDataCenter(0.f);
const float g_VoxelDataSize(0.5f);
const float g_VoxelDataSize_rcp = 1.f / g_VoxelDataSize;
const int g_VoxelDataResi_rcp = 32; // perf drop after 128
const float g_VoxelDataRes_rcp = float(g_VoxelDataResi_rcp);
const float g_VoxelDataRes(1.f / g_VoxelDataRes_rcp);

glm::vec3 projectFromTextureCoord(const glm::vec3& texCoord)
{
    assert(0.f <= texCoord.x && 1.f >= texCoord.x);
    assert(0.f <= texCoord.y && 1.f >= texCoord.y);
    assert(0.f <= texCoord.z && 1.f >= texCoord.z);
    glm::vec3 worldCoord = texCoord - 0.5f;
    worldCoord /= glm::vec3(0.5f, -0.5f, 0.5f);
    worldCoord *= g_VoxelDataSize;
    worldCoord += g_VoxelDataCenter;
    return worldCoord;
}

glm::vec3 projectToTextureCoord(const glm::vec3& worldCoord)
{
    glm::vec3 tc = worldCoord;
    tc = (tc - g_VoxelDataCenter);
    tc *= g_VoxelDataSize_rcp;
    tc = tc * glm::vec3(0.5f, -0.5f, 0.5f) + 0.5f;
    assert(0.f <= tc.x && 1.f >= tc.x);
    assert(0.f <= tc.y && 1.f >= tc.y);
    assert(0.f <= tc.z && 1.f >= tc.z);
    return tc;
}

void SDFRenderer::Render(const Scene * scene)
{
    mShaderProgram->Bind();

    if (0 == mVoxelTexId)
    {
        //create grid of random numbers:
        const int tex_width = g_VoxelDataResi_rcp;
        const int tex_height = g_VoxelDataResi_rcp;
        const int tex_depth = g_VoxelDataResi_rcp;
        const int tex_size = tex_width * tex_height * tex_depth;
        mVoxelData.reset(new GLfloat[tex_size]);
        GLfloat* noise_data = mVoxelData.get();

        for (int d = 0; d < tex_depth; ++d)
        for (int h = 0; h < tex_height; ++h)
        for (int w = 0; w < tex_width; ++w)
        {
            const int i = w + (h * tex_width) + (d * tex_width * tex_height);
            const glm::vec3 texCoord = glm::vec3(w, h, d) * g_VoxelDataRes;
            const glm::vec3 worldPosition = projectFromTextureCoord(texCoord);
            assert(glm::length(texCoord - projectToTextureCoord(worldPosition)) < 0.1f);
            const float sphere1 = sdSphere(worldPosition, g_VoxelDataSize * 0.5f);
            const float sphere2 = sdSphere(worldPosition + glm::vec3(g_VoxelDataSize * 0.5f, 0, 0), g_VoxelDataSize * 0.2f);
            noise_data[i] = glm::min(sphere1, sphere2);
            //noise_data[i] = sdSphere(worldPosition, g_VoxelDataSize * 0.5f);
            //noise_data[i] = sdBox(worldPosition, glm::vec3(g_VoxelDataSize * 0.25));
        }

        //create and bind texture
        glGenTextures(1, &mVoxelTexId);
        glBindTexture(GL_TEXTURE_3D, mVoxelTexId);

        //set filtering and wrapping
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, tex_width, tex_height, tex_depth, 0, GL_RED, GL_FLOAT, nullptr);
    }

    glBindTexture(GL_TEXTURE_3D, mVoxelTexId);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, g_VoxelDataResi_rcp, g_VoxelDataResi_rcp, g_VoxelDataResi_rcp, GL_RED, GL_FLOAT, mVoxelData.get());

    const Camera* camera = Root::Instance().GetCamera();
    glm::vec3 p;
    glm::quat q;
    camera->GetTransform(p, q);

    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("camera"));
        const glm::mat3 ca(q);

        glm::mat3 vRot(glm::rotate(glm::mat4(), Constant::Rotation * glm::pi<float>() * 2.f, glm::vec3(0, 1, 0)));

        vRot = ca * glm::inverse(vRot);

        glUniformMatrix3fv(uniformID, 1, false, glm::value_ptr(vRot));
    }

    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("ro"));
        glUniform3fv(uniformID, 1, glm::value_ptr(p));
    }

    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("iResolution"));
        glm::vec2 screenSize(camera->ScreenSize());
        glUniform2fv(uniformID, 1, glm::value_ptr(screenSize));
    }


    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("iVoxelDataCenter"));
        glUniform3fv(uniformID, 1, glm::value_ptr(g_VoxelDataCenter));
    }

    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("iVoxelDataSize"));
        glUniform1fv(uniformID, 1, &g_VoxelDataSize);
    }

    {
        GLuint uniformID = mShaderProgram->GetUniformLocation(HashedString("iVoxelDataSize_rcp"));
        glUniform1fv(uniformID, 1, &g_VoxelDataSize_rcp);
    }

    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("vPosition"));
        glBindBuffer(GL_ARRAY_BUFFER, mVertices.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndex.mVboId);
    glDrawElements(GL_TRIANGLES, mIndex.mElements.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mShaderProgram->Unbind();
}

void SDFRenderer::FlushFrame()
{
}

void SDFRenderer::ListResources(std::vector<Resource*>& resources)
{
    resources.push_back(mShaderResource.get());
}

void SDFRenderer::OnLoad()
{
    mShaderProgram = mShaderResource->mShaderProgram;
}
