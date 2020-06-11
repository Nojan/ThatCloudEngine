#pragma once

#include "config.hpp"
#include "color.hpp"
#include "mesh_renderer.hpp"
#include "opengl_includes.hpp"

#include <glm/fwd.hpp>
#include <memory>

class ShaderProgram;
class ResourceShader;
class RenderableMesh;

class ShadowRenderer : public GenericMeshRenderer
{
public:
    ShadowRenderer();
    virtual ~ShadowRenderer() = default;

    void Render(const Scene* scene) override;
    void FlushFrame() override;

    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    MeshBuffer* RequestMeshBuffer(uint32_t vertexCount, uint32_t indexCount = 0) override;

    void PushToRenderQueue(std::shared_ptr<RenderableMesh>& renderable);

#if GUI_DEBUG()
    void debug_GUI() const override {};
#endif
    const char* debug_name() const  override { return "ShadowMapRender"; };
private:
    void Render(const RenderableMesh& renderable, const glm::mat4& depthVP);

private:
    std::unique_ptr<ResourceShader> mShaderResource;
    std::vector<std::shared_ptr<RenderableMesh>> mRenderQueue;
};
