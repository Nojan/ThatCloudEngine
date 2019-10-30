#include "billboard_renderer.hpp"

#include "camera.hpp"
#include "global.hpp"
#include "opengl_helpers.hpp"
#include "shader.hpp"
#include "resourcemanager.hpp"
#include "shader_loader.hpp"
#include "texture.hpp"
#include "root.hpp"

#include "imgui/imgui_header.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cassert>

#ifdef IMGUI_ENABLE
void BillboardRenderer::debug_GUI() const {

}
#endif

BillboardRenderer::BillboardRenderer()
: mVboVerticesId(0)
, mVboNormalId(0)
, mVboTexCoordId(0)
, mVboIndexId(0)
, mTextureId(0)
{
    mShaderProgram = Global::resourceManager()->shader("billboard");
    mShaderProgram->RegisterAttrib(HashedString("vertexPosition_modelspace"));
    mShaderProgram->RegisterAttrib(HashedString("textureCoord"));
    mShaderProgram->RegisterUniform(HashedString("alpha"));
    mShaderProgram->RegisterUniform(HashedString("mvp"));
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboVerticesId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec3>(4, &mVboNormalId);
    generate_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW, glm::vec2>(4, &mVboTexCoordId);
    generate_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW, uint>(6, &mVboIndexId);
    glGenTextures(1, &mTextureId);
}

BillboardRenderer::~BillboardRenderer()
{
    glDeleteBuffers(1, &mVboVerticesId);
    glDeleteBuffers(1, &mVboNormalId);
    glDeleteBuffers(1, &mVboTexCoordId);
    glDeleteBuffers(1, &mVboIndexId);
    glDeleteTextures(1, &mTextureId);
}

void BillboardRenderer::PushToRenderQueue(const Billboard& billboard)
{
    mRenderQueue.push_back(billboard);
}

void BillboardRenderer::Render(const Scene * scene)
{
	if (mRenderQueue.empty())
        return;
	glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    mShaderProgram->Bind();
	SortQueue();

    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 direction = glm::normalize(camera->Direction() * -1.f);
    const glm::vec3& up = camera->Up();
    const glm::vec3 ortoDirection = glm::cross(direction, up);

	for (const Billboard& billboard: mRenderQueue)
    {
        Render(billboard, direction, up, ortoDirection);
    }
    mShaderProgram->Unbind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    mRenderQueue.clear();
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

    const glm::vec3 sizeX = size.x * up * 0.5f;
    const glm::vec3 sizeY = size.y * ortoDirection * 0.5f;

    std::vector<glm::vec3> vertices = { -sizeX -sizeY, sizeX -sizeY, -sizeX + sizeY, sizeX + sizeY};
    for (size_t idx = 0; idx < 4; ++idx)
    {
        vertices[idx] += position;
    }

    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(vertices, mVboVerticesId); 
    std::vector<glm::vec3> normals = { normal, normal, normal, normal };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(normals, mVboNormalId); 
    std::vector<glm::vec2> texCoord = { glm::vec2(0.01, 0.99), glm::vec2(0.99, 0.99), glm::vec2(0.01, 0.01), glm::vec2(0.99, 0.01) };
    update_gl_array_buffer<GL_ARRAY_BUFFER, GL_STREAM_DRAW>(texCoord, mVboTexCoordId); 
    std::vector<uint> index = { 0, 1, 2, 2, 1, 3 };
    update_gl_array_buffer<GL_ELEMENT_ARRAY_BUFFER, GL_STREAM_DRAW>(index, mVboIndexId);
    {
        GLuint uniform_ID = mShaderProgram->GetUniformLocation(HashedString("alpha"));
        glUniform1f(uniform_ID, alpha);
    }
    {
        GLuint matrixMVP_ID = mShaderProgram->GetUniformLocation(HashedString("mvp"));
        glm::mat4 mvp = Root::Instance().GetCamera()->ProjectionView();
        glUniformMatrix4fv(matrixMVP_ID, 1, GL_FALSE, glm::value_ptr(mvp));
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("vertexPosition_modelspace"));
        glBindBuffer(GL_ARRAY_BUFFER, mVboVerticesId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    {
        GLuint attributeID = mShaderProgram->GetAttribLocation(HashedString("textureCoord"));
        glBindBuffer(GL_ARRAY_BUFFER, mVboTexCoordId);
        glEnableVertexAttribArray(attributeID);
        glVertexAttribPointer(attributeID, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    const std::shared_ptr< Texture2D >& texture = billboard.mTexture;
    GPUBufferHandle& bufferHandle = texture->BufferHandle();
    glActiveTexture(GL_TEXTURE0);
    if (bufferHandle.valid())
    {
        glBindTexture(GL_TEXTURE_2D, bufferHandle.Id());
    }
    else
    {
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
    // attribute buffer : index
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mVboIndexId);
    glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, 0);
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
}