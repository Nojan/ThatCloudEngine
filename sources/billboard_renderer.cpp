#include "billboard_renderer.hpp"

#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "shader.hpp"
#include "resourceshader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cassert>

#if GUI_DEBUG()
void BillboardRenderer::debug_GUI() const {

}
#endif

BillboardRenderer::BillboardRenderer()
{
    mShaderResource = std::make_unique<ResourceShader>("billboard");
    mShaderResource->PreloadAttribute(HashedString("vertexPosition_modelspace"));
    mShaderResource->PreloadAttribute(HashedString("textureCoord"));
    mShaderResource->PreloadAttribute(HashedString("a_alpha"));
    mShaderResource->PreloadAttribute(HashedString("a_textureIndex"));
    mShaderResource->PreloadUniform(HashedString("mvp"));
    mShaderResource->PreloadUniform(HashedString("textureSampler"));
    FlushFrame();
}

BillboardRenderer::~BillboardRenderer()
{
}

void BillboardRenderer::PushToRenderQueue(const Billboard& billboard)
{
    size_t textureId = -1;
    for (size_t idx = 0; -1 == textureId && idx < mTextures.size(); ++idx)
    {
        if (mTextures[idx] == billboard.mTexture)
        {
            textureId = idx;
        }
        if (mTextures[idx] == nullptr)
        {
            mTextures[idx] = billboard.mTexture;
            textureId = idx;
        }
    }
    assert(-1 != textureId);
    mRenderQueue.push_back(billboard);
}

void BillboardRenderer::Render(const Scene * scene)
{
	if (mRenderQueue.empty())
        return;
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mShaderProgram->Bind();
	SortQueue();

    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 direction = glm::normalize(camera->Direction() * -1.f);
    const glm::vec3& up = camera->Up();
    const glm::vec3 ortoDirection = glm::cross(direction, up);

    for (size_t idx = 0; idx < mTextures.size(); ++idx)
    {
        std::shared_ptr< Texture2D >& texture = mTextures[idx];
        if (nullptr == texture)
        {
            continue;
        }
        glActiveTexture(GL_TEXTURE0 + idx);
        GPUBufferHandle& bufferHandle = texture->BufferHandle();
        if (bufferHandle.valid())
        {
            glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
            continue;
        }
        GLuint id;
        glGenTextures(1, &id);
        bufferHandle.setId(id);
        glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ColorsChannel cchannel = texture->colorChannel();
        assert(ColorsChannel::RGB == cchannel || ColorsChannel::RGBA == cchannel);
        if(ColorsChannel::RGB == cchannel)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->getWidth(), texture->getHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, texture->getData());
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->getWidth(), texture->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->getData());
    }
    {
        const GLint textureSampler_ID = mShaderProgram->GetUniformLocation(HashedString("textureSampler"));
        std::array<GLint, 8> textureUnit = {0, 1, 2, 3, 4, 5, 6, 7};
        glUniform1iv(textureSampler_ID, textureUnit.size(), textureUnit.data()); 
    }

    const size_t billboardCount = mRenderQueue.size();
    mVertices.mElements.reserve(4*billboardCount);
    mTexCoords.mElements.reserve(4*billboardCount);
    mAlpha.mElements.reserve(4*billboardCount);
    mTexIdx.mElements.reserve(4*billboardCount);
    mIndex.mElements.reserve(6*billboardCount);

	for (const Billboard& billboard: mRenderQueue)
    {
        Render(billboard, direction, up, ortoDirection);
    }

    mVertices.StreamGPU();
    mTexCoords.StreamGPU();
    mAlpha.StreamGPU();
    mTexIdx.StreamGPU();
    mIndex.StreamGPU();

    {
        GLuint matrixMVP_ID = mShaderProgram->GetUniformLocation(HashedString("mvp"));
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("vertexPosition_modelspace"));
        glBindBuffer(GL_ARRAY_BUFFER, mVertices.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("textureCoord"));
        glBindBuffer(GL_ARRAY_BUFFER, mTexCoords.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("a_alpha"));
        glBindBuffer(GL_ARRAY_BUFFER, mAlpha.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("a_textureIndex"));
        glBindBuffer(GL_ARRAY_BUFFER, mTexIdx.mVboId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 1, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndex.mVboId);
    glDrawElements(GL_TRIANGLES, mIndex.mElements.size(), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("vertexPosition_modelspace"));
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("textureCoord"));
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("a_alpha"));
        glDisableVertexAttribArray(attributeID);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("a_textureIndex"));
        glDisableVertexAttribArray(attributeID);
    }

    mShaderProgram->Unbind();
    glDisable(GL_BLEND);
}

void BillboardRenderer::FlushFrame()
{
    mRenderQueue.clear();
    for (size_t idx = 0; idx < mTextures.size(); ++idx)
    {
        mTextures[idx] = nullptr;
    }
    mVertices.mElements.clear();
    mTexCoords.mElements.clear();
    mAlpha.mElements.clear();
    mTexIdx.mElements.clear();
    mIndex.mElements.clear();
}

void BillboardRenderer::ListResources(std::vector<Resource*>& resources)
{
    resources.push_back(mShaderResource.get());
}

void BillboardRenderer::OnLoad()
{
    mShaderProgram = mShaderResource->mShaderProgram;
}

void BillboardRenderer::SortQueue()
{
    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3& position = camera->Position();
    const glm::vec3& normal = camera->Direction();
    
    std::sort(mRenderQueue.begin(), mRenderQueue.end(),
        [&position, &normal](const Billboard& a, const Billboard& b) -> bool
    {
        const float dst_a = glm::dot(normal, position - a.mPosition);
        const float dst_b = glm::dot(normal, position - b.mPosition);
        return dst_a < dst_b;
    });
}

void BillboardRenderer::Render(const Billboard& billboard, const glm::vec3& direction, const glm::vec3& up, const glm::vec3& ortoDirection)
{
    const glm::vec3 position = billboard.mPosition;
    const glm::vec3 normal = direction;
    const glm::vec2 size = billboard.mSize;
    const float alpha = billboard.mAlpha;

    size_t textureId = -1;
    for (size_t idx = 0; idx < mTextures.size(); ++idx)
    {
        if (mTextures[idx] == billboard.mTexture)
        {
            textureId = idx;
            break;
        }
    }
    assert(-1 != textureId);
    const float textureIdx(textureId);

    const glm::vec3 sizeX = size.x * up * 0.5f;
    const glm::vec3 sizeY = size.y * ortoDirection * 0.5f;

    std::array<glm::vec3, 4> vertices = { -sizeX -sizeY, sizeX -sizeY, -sizeX + sizeY, sizeX + sizeY};
    for (size_t idx = 0; idx < vertices.size(); ++idx)
    {
        vertices[idx] += position;
    }

    const uint current_index = numeric_cast<uint>(mVertices.mElements.size());
    mVertices.mElements.insert(mVertices.mElements.end(), vertices.begin(), vertices.end());
    
    std::array<glm::vec2, 4> texCoord = { glm::vec2(0.01, 0.99), glm::vec2(0.99, 0.99), glm::vec2(0.01, 0.01), glm::vec2(0.99, 0.01) };
    mTexCoords.mElements.insert(mTexCoords.mElements.end(), texCoord.begin(), texCoord.end());
    
    std::array<float, 4> alphas = {alpha, alpha, alpha, alpha};
    mAlpha.mElements.insert(mAlpha.mElements.end(), alphas.begin(), alphas.end());

    std::array<float, 4> textureIndices = {textureIdx, textureIdx, textureIdx, textureIdx};
    mTexIdx.mElements.insert(mTexIdx.mElements.end(), textureIndices.begin(), textureIndices.end());

    std::array<uint, 6> index = { 0, 1, 2, 2, 1, 3 };
    for (size_t idx = 0; idx < index.size(); ++idx)
    {
        index[idx] += current_index;
    }
    mIndex.mElements.insert(mIndex.mElements.end(), index.begin(), index.end());
}
