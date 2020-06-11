#include "shadowRenderer.hpp"


#include "camera.hpp"
#include "HashedString.hpp"
#include "irenderer.hpp"
#include "mesh_buffer_gpu.hpp"
#include "renderableMesh.hpp"
#include "resourceshader.hpp"
#include "root.hpp"
#include "scene.hpp"
#include "shader.hpp"

#include "imgui/imgui_header.hpp"

#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

ShadowRenderer::ShadowRenderer()
{
    mShaderResource = std::make_unique<ResourceShader>("shadow");
    mShaderResource->PreloadAttribute(HashedString("Position"));
    mShaderResource->PreloadUniform(HashedString("mvp"));
}

void ShadowRenderer::Render(const Scene* scene)
{
    if (mRenderQueue.empty())
        return;
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    mShaderProgram->Bind();

    const glm::vec3 lightInvDir = glm::normalize(scene->GetDirectionalLight().mDirection);

    const glm::mat4 depthProjectionMatrix = glm::ortho<float>(-15, 15, -15, 15, -15, 15);
    const glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    const glm::mat4 depthVP = depthProjectionMatrix * depthViewMatrix;

    for (const std::shared_ptr<RenderableMesh>& renderable : mRenderQueue)
    {
        assert(renderable);
        Render(*renderable, depthVP);
    }

    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
}

void ShadowRenderer::FlushFrame()
{
    mRenderQueue.clear();
}

void ShadowRenderer::ListResources(std::vector<Resource*>& resources)
{
    resources.push_back(mShaderResource.get());
}

void ShadowRenderer::OnLoad()
{
    mShaderProgram = mShaderResource->mShaderProgram;
}

MeshBuffer* ShadowRenderer::RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount)
{
    assert(false);
    return nullptr;
}

void ShadowRenderer::PushToRenderQueue(std::shared_ptr<RenderableMesh>& renderable)
{
    assert(renderable->mMesh->Valid());
    MeshBufferGpu* meshBuffer = dynamic_cast<MeshBufferGpu*>(renderable->mMeshBuffer.get());
    assert(meshBuffer);
    mRenderQueue.push_back(renderable);
}

void ShadowRenderer::Render(const RenderableMesh& renderable, const glm::mat4& depthVP)
{
    if (renderable.mMesh->mIndex.empty())
        return;
    const glm::mat4 modelTransformAndScale = renderable.mTransform * renderable.mScale;
    {
        GLint matrixMVP_ID = mShaderProgram->GetUniformLocation(HashedString("mvp"));
        glm::mat4 mvp = depthVP * modelTransformAndScale;
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    const size_t positionIndex = renderable.mMeshBuffer->Layout().GetIndex(VertexSemantic::Position);
    GenericMeshRenderer::PrepareAttribute(renderable.mMeshBuffer.get(), numeric_cast<uint16_t>(positionIndex));
    GenericMeshRenderer::Draw(renderable.mMeshBuffer.get());
}
