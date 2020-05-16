#include "render_manager.hpp"

#include "HashedString.hpp"
#include "irenderer.hpp"
#include "resourceshader.hpp"
#include "shader.hpp"
#include "opengl_includes.hpp"
#include "vbo.hpp"
#include "imgui/imgui_header.hpp"

#include <cassert>
#include <glm/glm.hpp>

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
private:
    std::unique_ptr<ResourceShader> mShaderResource;
    std::shared_ptr<ShaderProgram> mShaderProgram;
    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mVertices;
    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mTexCoords;
};

struct RenderTarget {
    unsigned int framebuffer = 0;
    unsigned int textureColorbuffer = 0; 
    unsigned int rbo = 0;
    bool init = false;
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
        glm::vec2(-1.0,  1.0),
        glm::vec2(-1.0, -1.0),
        glm::vec2( 1.0, -1.0),

        glm::vec2(-1.0,  1.0),
        glm::vec2( 1.0, -1.0),
        glm::vec2( 1.0,  1.0),
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
{
}

RenderManager::~RenderManager()
{
}

void RenderManager::Render(const Scene* scene)
{
#ifdef __EMSCRIPTEN__
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& renderer : mRendererList)
    {
        renderer->Render(scene);
    }
#else
    static RenderTarget target;
    if (false == target.init)
    {
        const int screen_width = 800;
        const int screen_height = 600;
        glGenFramebuffers(1, &target.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
        glGenTextures(1, &target.textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, target.textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screen_width, screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.textureColorbuffer, 0);
        assert(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER));
        target.init = true;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, target.framebuffer);
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, target.textureColorbuffer);
    for (auto& renderer : mRendererList)
    {
        renderer->Render(scene);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.f, 1.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, target.textureColorbuffer);
    glDisable(GL_DEPTH_TEST);
    mFinalRender->Render(scene);
#endif
}

void RenderManager::FlushFrame()
{
    for (auto& renderer : mRendererList)
    {
        renderer->FlushFrame();
    }
    mFinalRender->FlushFrame();
}

void RenderManager::ListResources(std::vector<Resource*>& resources)
{
    for (auto& renderer : mRendererList)
    {
        renderer->ListResources(resources);
    }
    mFinalRender->ListResources(resources);
}

void RenderManager::OnLoad()
{
    for (auto& renderer : mRendererList)
    {
        renderer->OnLoad();
    }
    mFinalRender->OnLoad();
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
    }
}
#endif