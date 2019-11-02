#pragma once

#include "billboard.hpp"
#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"
#include "vbo.hpp"

#include <array>
#include <memory>
#include <vector>

class ShaderProgram;
class Texture2D;

class BillboardRenderer : public IRenderer {
public:

    void Render(const Scene * scene) override;
    void FlushFrame() override;

    BillboardRenderer();
    ~BillboardRenderer();

    void PushToRenderQueue(const Billboard& billboard);

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "Billboard Renderer"; }

private:
    void Render(const Billboard& billboard, const glm::vec3& direction, const glm::vec3& up, const glm::vec3& ortoDirection);
    void SortQueue();

private:
    std::shared_ptr<ShaderProgram> mShaderProgram;
    std::vector<Billboard> mRenderQueue;
    std::array<std::shared_ptr< Texture2D >, 8> mTextures;

    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec3> mVertices;
    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mTexCoords;
    VBO_dynamic<GL_ARRAY_BUFFER, float> mAlpha;
    VBO_dynamic<GL_ARRAY_BUFFER, float> mTexIdx;
    VBO_dynamic<GL_ELEMENT_ARRAY_BUFFER, uint> mIndex;
};
