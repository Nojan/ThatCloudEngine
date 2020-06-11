#include "render_manager.hpp"

#include "global.hpp"
#include "HashedString.hpp"
#include "irenderer.hpp"
#include "meshRenderer.hpp"
#include "renderer_list.hpp"
#include "resourceshader.hpp"
#include "shader.hpp"
#include "shadowRenderer.hpp"
#include "opengl_includes.hpp"
#include "vbo.hpp"
#include "imgui/imgui_header.hpp"

#include <cassert>
#include <glm/glm.hpp>

struct RenderTarget {
    unsigned int framebuffer = 0;
    unsigned int textureID = 0;
    unsigned int renderBufferID = 0;
    bool init = false;
};

class FinalRender : public IRenderer
{
public:
    FinalRender();
    virtual ~FinalRender() = default;

    void Render(const Scene* scene) override;
    void FlushFrame() override {};

    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

#if GUI_DEBUG()
    void debug_GUI() const override {};
#endif
    const char* debug_name() const  override { return "FinalRender"; };
    unsigned int mTextureID = 0;
private:
    std::unique_ptr<ResourceShader> mShaderResource;
    std::shared_ptr<ShaderProgram> mShaderProgram;
    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mVertices;
    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mTexCoords;
};

FinalRender::FinalRender()
{
    mShaderResource = std::make_unique<ResourceShader>("screen");
    mShaderResource->PreloadAttribute(HashedString("aPosition"));
    mShaderResource->PreloadAttribute(HashedString("aTexCoord"));
    mShaderResource->PreloadUniform(HashedString("textureSampler"));

    mVertices.mElements.reserve(12);
    mTexCoords.mElements.reserve(12);

    mVertices.mElements = { 
        glm::vec2(-1.0, 1.0),
        glm::vec2(-1.0, 0.6),
        glm::vec2(-0.6, 0.6),

        glm::vec2(-1.0, 1.0),
        glm::vec2(-0.6, 0.6),
        glm::vec2(-0.6, 1.0),
    };

    mTexCoords.mElements = {
        glm::vec2(0.0, 1.0),
        glm::vec2(0.0, 0.0),
        glm::vec2(1.0, 0.0),

        glm::vec2(0.0, 1.0),
        glm::vec2(1.0, 0.0),
        glm::vec2(1.0, 1.0),
    };

    mVertices.StreamGPU();
    mTexCoords.StreamGPU();
}

void FinalRender::Render(const Scene* scene)
{
    mShaderProgram->Bind();
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("aPosition"));
        glBindBuffer(GL_ARRAY_BUFFER, mVertices.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("aTexCoord"));
        glBindBuffer(GL_ARRAY_BUFFER, mTexCoords.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureID);
        GLint textureSampler_ID = mShaderProgram->GetUniformLocation(HashedString("textureSampler"));
        glUniform1i(textureSampler_ID, 0);
    }
    glDrawArrays(GL_TRIANGLES, 0, 6);
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("aPosition"));
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("aTexCoord"));
        glDisableVertexAttribArray(attributeID);
    }
    mShaderProgram->Unbind();
}

void FinalRender::ListResources(std::vector<Resource*>& resources)
{
    resources.push_back(mShaderResource.get());
}

void FinalRender::OnLoad()
{
    mShaderProgram = mShaderResource->mShaderProgram;
}

RenderManager::RenderManager()
: mFinalRender(std::make_unique<FinalRender>())
, mShadowRender(std::make_unique<ShadowRenderer>())
{
    Global::rendererList()->addRenderer(mShadowRender.get());
}

RenderManager::~RenderManager()
{
}

void RenderManager::Render(const Scene* scene, int screen_width, int screen_height)
{
    static RenderTarget target;
    //screen_width = 1024;
    //screen_height = 1024;
    const int shadowMapSize = 1024;
    if (false == target.init)
    {
        glGenFramebuffers(1, &target.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glGenTextures(1, &target.textureID);
        glBindTexture(GL_TEXTURE_2D, target.textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.textureID, 0);
        assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
        target.init = true;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    static RenderTarget shadow_map;
    if (false == shadow_map.init)
    {
        glGenFramebuffers(1, &shadow_map.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.framebuffer);
        glGenTextures(1, &shadow_map.textureID);
        glBindTexture(GL_TEXTURE_2D, shadow_map.textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, shadowMapSize, shadowMapSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shadow_map.textureID, 0);

        if (false)
        {
            glGenTextures(1, &shadow_map.renderBufferID);
            glBindTexture(GL_TEXTURE_2D, shadow_map.renderBufferID);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            //glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
            //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow_map.renderBufferID, 0);
        }
        else
        {
            glGenRenderbuffers(1, &shadow_map.renderBufferID);
            glBindRenderbuffer(GL_RENDERBUFFER, shadow_map.renderBufferID);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, shadowMapSize, shadowMapSize);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, shadow_map.renderBufferID);
        }



        assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
        shadow_map.init = true;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map.framebuffer);
    glBindTexture(GL_TEXTURE_2D, shadow_map.textureID);
    glBindRenderbuffer(GL_RENDERBUFFER, shadow_map.renderBufferID);
    glViewport(0, 0, shadowMapSize, shadowMapSize);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    mShadowRender->Render(scene);

    Global::rendererList()->getRenderer<MeshRenderer>()->mShadowMap = shadow_map.textureID;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screen_width, screen_height);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& renderer : mRendererList)
    {
        renderer->Render(scene);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_map.textureID);
    glDisable(GL_DEPTH_TEST);
    mFinalRender->mTextureID = shadow_map.textureID;
    mFinalRender->Render(scene);
}

void RenderManager::FlushFrame()
{
    for (auto& renderer : mRendererList)
    {
        renderer->FlushFrame();
    }
    mFinalRender->FlushFrame();
    mShadowRender->FlushFrame();
}

void RenderManager::ListResources(std::vector<Resource*>& resources)
{
    for (auto& renderer : mRendererList)
    {
        renderer->ListResources(resources);
    }
    mFinalRender->ListResources(resources);
    mShadowRender->ListResources(resources);
}

void RenderManager::OnLoad()
{
    for (auto& renderer : mRendererList)
    {
        renderer->OnLoad();
    }
    mFinalRender->OnLoad();
    mShadowRender->OnLoad();
}

#if GUI_DEBUG()
void RenderManager::debug_GUI() const
{
    if (ImGui::CollapsingHeader("Renderer"))
    {
        for (auto& renderer : mRendererList)
        {
            if (ImGui::CollapsingHeader(renderer->debug_name()))
                renderer->debug_GUI();
        }
        if (ImGui::CollapsingHeader(mFinalRender->debug_name()))
            mFinalRender->debug_GUI();
        if (ImGui::CollapsingHeader(mShadowRender->debug_name()))
            mShadowRender->debug_GUI();
    }
}
#endif