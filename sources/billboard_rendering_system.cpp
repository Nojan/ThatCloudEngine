#include "billboard_rendering_system.hpp"

#include "camera.hpp"
#include "billboard_renderer.hpp"
#include "global.hpp"
#include "root.hpp"
#include "renderer_list.hpp"
#include "transform_system.hpp"

#include <Tracy/Tracy.hpp>
#include <glm/gtc/quaternion.hpp>

void BillboardComponent::draw(BillboardRenderer * renderer)
{
    if(!mEnable)
        return;
    const glm::vec3 worldPosition(mTransformComponent->Position());
    for (const Billboard& b : mBillboards)
    {
        if(b.mAlpha <= 0)
            continue;
        Billboard billboard = b;
        billboard.mPosition += worldPosition;
        renderer->PushToRenderQueue(billboard);
    }
}

void BillboardRenderingSystem::FrameStep()
{
    ZoneScoped;
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<BillboardRenderer>();
    assert(mRenderer);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer);
    }
}

void BillboardRenderingSystem::attachEntity(GameEntity * entity)
{
    BillboardComponent* component = IComponentSystem::attachPtrComponent<BillboardComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component->mTransformComponent = tranform;
}

void BillboardRenderingSystem::detachEntity(GameEntity * entity)
{
    IComponentSystem::detachPtrComponent<BillboardComponent>(entity, mComponents);
}
