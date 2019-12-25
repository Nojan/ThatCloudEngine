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

#if GUI_DEBUG()
void SDFRenderer::debug_GUI() const {

}
#endif

SDFRenderer::SDFRenderer()
{
    mShaderResource = std::make_unique<ResourceShader>("sdf");
    mShaderResource->PreloadAttribute(HashedString("vPosition"));
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

void SDFRenderer::Render(const Scene * scene)
{
    mShaderProgram->Bind();

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
