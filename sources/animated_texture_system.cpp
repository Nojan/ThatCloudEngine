#include "animated_texture_system.hpp"

#include "rendering_system.hpp"
#include "renderableMesh.hpp"

AnimatedTextureComponent::AnimatedTextureComponent()
: mGraphicComponent(nullptr)
, mIdx(0)
{
}

void AnimatedTextureComponent::Update(const float deltaTime)
{
    const size_t nextIdx = (mIdx + 1) % mTexture.size();
    for (std::unique_ptr<RenderableMesh>& renderable : mGraphicComponent->mRenderable)
    {
        if (renderable->mMaterial.Texture() == mTexture[mIdx])
        {
            renderable->mMaterial.Texture() = mTexture[nextIdx];
        }
    }
    mIdx = nextIdx;
}

void AnimatedTextureSystem::Update(const float deltaTime)
{
    for (std::unique_ptr<AnimatedTextureComponent>& compoment : mComponents)
    {
        compoment->Update(deltaTime);
    }
}

void AnimatedTextureSystem::attachEntity(GameEntity * entity)
{
    AnimatedTextureComponent* component = IComponentSystem::attachPtrComponent<AnimatedTextureComponent>(entity, mComponents);
    GraphicMeshComponent* graphic = entity->getComponent<GraphicMeshComponent>();
    assert(graphic);
    component->mGraphicComponent = graphic;
}

void AnimatedTextureSystem::detachEntity(GameEntity * entity)
{
    IComponentSystem::detachPtrComponent<AnimatedTextureComponent>(entity, mComponents);
}
