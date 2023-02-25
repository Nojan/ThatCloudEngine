#include "animated_texture_system.hpp"

#include "rendering_system.hpp"
#include "renderableMesh.hpp"

#include "imgui/imgui_header.hpp"
#include <tracy/Tracy.hpp>

namespace Constant {
IMGUI_VAR(AnimatedTextureTimer, 0.1f);
}

#if GUI_DEBUG()
void AnimatedTextureSystem::debug_GUI() const
{
    ImGui::SliderFloat("Timer", &Constant::AnimatedTextureTimer, 0.f, 0.2f);
}
#endif

AnimatedTextureComponent::AnimatedTextureComponent()
: mGraphicComponent(nullptr)
, mIdx(0)
, mTimer(Constant::AnimatedTextureTimer)
{
}

void AnimatedTextureComponent::Update(const float deltaTime)
{
    mTimer -= deltaTime;
    if(0 < mTimer)
        return;
    mTimer = Constant::AnimatedTextureTimer;
    const size_t nextIdx = (mIdx + 1) % mTexture.size();
    for (std::shared_ptr<RenderableMesh>& renderable : mGraphicComponent->mRenderable)
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
    ZoneScopedN("AnimatedTextureSystem::Update");
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
