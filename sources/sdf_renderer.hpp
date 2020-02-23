#pragma once

#include "config.hpp"
#include "color.hpp"
#include "irenderer.hpp"
#include "types.hpp"
#include "vbo.hpp"

#include <glm/glm.hpp>

#include <array>
#include <memory>
#include <vector>

class ShaderProgram;
class ResourceShader;

class SDFRenderer : public IRenderer {
public:

    void Render(const Scene * scene) override;
    void FlushFrame() override;
    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    SDFRenderer();
    ~SDFRenderer();

#if GUI_DEBUG()
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "SDF Renderer"; }

private:
    std::unique_ptr<ResourceShader> mShaderResource;
    std::shared_ptr<ShaderProgram> mShaderProgram;

    GLuint mVoxelTexId = 0;
    std::unique_ptr<GLfloat[]> mVoxelData;

    VBO_dynamic<GL_ARRAY_BUFFER, glm::vec2> mVertices;
    VBO_dynamic<GL_ELEMENT_ARRAY_BUFFER, uint> mIndex;
};
