#include "rendering_system.hpp"

#include "game_entity.hpp"
#include "renderer_list.hpp"
#include "renderableMesh.hpp"
#include "renderableSkinMesh.hpp"
#include "transform_system.hpp"
#include "visualdebug.hpp"

#include "tracy_helper.hpp"
#include <cassert>

#include "global.hpp"
#include "meshRenderer.hpp"
#include "skinMeshRenderer.hpp"
#include "armature.hpp"

void GraphicMeshComponent::draw(MeshRenderer* renderer)
{
    if(mRenderable.empty() || !mEnable)
        return;

    for (std::shared_ptr<RenderableMesh>& renderable: mRenderable)
    {
        renderable->mTransform = mTransformComponent->Transform();
        renderable->mScale = mTransformComponent->mScale;
        //renderable->mScale = glm::mat4(5.f);
        renderable->mScale[3][3] = 1.f;
        renderer->PushToRenderQueue(renderable);
    }

}

BoundingBox3D GraphicMeshComponent::getLocalBoundingBox() const
{
    BoundingBox3D result;
    for (const std::shared_ptr<RenderableMesh>& renderable : mRenderable)
    {
        const BoundingBox3D& bbox = renderable->mMesh->mBBox;
        result.Add(bbox.Min());
        result.Add(bbox.Max());
    }
    return result;
}

BoundingBox3D GraphicMeshComponent::getBoundingBox() const
{
    const BoundingBox3D localAABB = getLocalBoundingBox();
    const glm::mat4 transform = mTransformComponent->Transform() * mTransformComponent->mScale;
    const glm::vec4 min(transform* glm::vec4(localAABB.Min(), 1));
    const glm::vec4 max(transform * glm::vec4(localAABB.Max(), 1));
    return BoundingBox3D(glm::vec3(min), glm::vec3(max));
}

void GraphicMeshComponent::setupResource(std::shared_ptr<MeshResourceList> resource)
{
    if(mResource == resource)
        return;
    
    mResource = resource;
    mRenderable.clear();

    if(!mResource)
        return;

    for (MeshResource& resource: *mResource)
    {
        std::unique_ptr<RenderableMesh> renderable = std::make_unique<RenderableMesh>();
        renderable->mMaterial.Texture() = resource.m_texture;
        renderable->mMesh = resource.m_mesh;
        mRenderable.push_back(std::move(renderable));
    }
}

RenderingSystem::RenderingSystem()
: mRenderer(nullptr)
{}

RenderingSystem::~RenderingSystem()
{}

void RenderingSystem::FrameStep()
{
    ZoneScopedN("RenderingSystem::Framestep");
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<MeshRenderer>();
    assert(mRenderer);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer);
    }
}

void RenderingSystem::attachEntity(GameEntity* entity) 
{
    GraphicMeshComponent* component = IComponentSystem::attachPtrComponent<GraphicMeshComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void RenderingSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachPtrComponent<GraphicMeshComponent>(entity, mComponents);
}

GraphicSkinComponent::GraphicSkinComponent()
    : mAnimationTime(0)
    , mAnimationRate(1.f)
    , mAnimationIdx(0)
{
}

void GraphicSkinComponent::draw(SkinMeshRenderer* renderer)
{
    if (!mRenderable || !mEnable)
        return;
    mRenderable->mTransform = mTransformComponent->Transform();
    mRenderable->mScale = mTransformComponent->mScale;
    mRenderable->mScale[3][3] = 1.f;
    const float animationLoopTime = mRenderable->mMesh->mArmature->animations[mAnimationIdx].duration;
    mAnimationTime = fmodf(mAnimationTime, animationLoopTime);
    mRenderable->mAnimationIdx = mAnimationIdx;
    mRenderable->mAnimationTime = mAnimationTime;
    renderer->PushToRenderQueue(mRenderable.get());
}

RenderingSkinSystem::RenderingSkinSystem()
    : mRenderer(nullptr)
{}

RenderingSkinSystem::~RenderingSkinSystem()
{}

void RenderingSkinSystem::FrameStep()
{
    ZoneScopedN("RenderingSkinSystem::Framestep");
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<SkinMeshRenderer>();
    assert(mRenderer);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer);
    }
}

void RenderingSkinSystem::Update(const float deltaTime)
{
    ZoneScopedN("RenderingSkinSystem::Update");
    for (auto& component : mComponents)
    {
        float time = component->mAnimationTime;
        const float rate = component->mAnimationRate;
        time += rate * deltaTime;
        component->mAnimationTime = time;
    }
}

void RenderingSkinSystem::attachEntity(GameEntity* entity)
{
    GraphicSkinComponent* component = IComponentSystem::attachPtrComponent<GraphicSkinComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void RenderingSkinSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachPtrComponent<GraphicSkinComponent>(entity, mComponents);
}
