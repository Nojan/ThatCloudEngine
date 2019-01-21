#pragma once

#include "icomponentsystem.hpp"
#include "igraphic_component.hpp"
#include "mesh_resource.hpp"
#include "color.hpp"
#include "types.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class MeshRenderer;
class RenderableMesh;
class SkinMeshRenderer;
class RenderableSkinMesh;
class TransformComponent;

class GraphicMeshComponent : public IGraphicComponent<MeshRenderer>
{
public:
    ~GraphicMeshComponent() = default;
    void draw(MeshRenderer* renderer);

    void setupResource(std::shared_ptr<MeshResourceList>& resource);

    std::vector<std::shared_ptr<RenderableMesh>> mRenderable;
    std::shared_ptr<MeshResourceList> mResource;
};

class RenderingSystem : public IComponentSystem {
public:
    RenderingSystem();
    virtual ~RenderingSystem();

    void FrameStep() override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "Rendering"; }

private:
    std::vector<std::unique_ptr<GraphicMeshComponent>> mComponents;
    MeshRenderer* mRenderer;
};

class GraphicSkinComponent : public IGraphicComponent<SkinMeshRenderer>
{
public:
    GraphicSkinComponent();
    ~GraphicSkinComponent() = default;
    void draw(SkinMeshRenderer* renderer);

    std::unique_ptr<RenderableSkinMesh> mRenderable;
    float mAnimationTime;
    float mAnimationRate;
    uint mAnimationIdx;
};

class RenderingSkinSystem : public IComponentSystem {
public:
    RenderingSkinSystem();
    virtual ~RenderingSkinSystem();

    void FrameStep() override;
    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "RenderingSkin"; }

private:
    std::vector<std::unique_ptr<GraphicSkinComponent>> mComponents;
    SkinMeshRenderer* mRenderer;
};