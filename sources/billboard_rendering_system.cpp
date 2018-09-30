#include "billboard_rendering_system.hpp"

#include "camera.hpp"
#include "billboard_renderer.hpp"
#include "global.hpp"
#include "root.hpp"
#include "renderer_list.hpp"
#include "transform_system.hpp"

#include <glm/gtc/quaternion.hpp>

void BillboardComponent::draw(BillboardRenderer * renderer, const glm::vec3& normal)
{
    if(!mEnable || mBillboard.mAlpha <= 0)
        return;
    mBillboard.mPosition = glm::vec3(mTransformComponent->Position());
    mBillboard.mNormal = normal;
    renderer->PushToRenderQueue(&mBillboard);
}

void BillboardRenderingSystem::FrameStep()
{
    if (!mRenderer)
        mRenderer = Global::rendererList()->getRenderer<BillboardRenderer>();
    assert(mRenderer);
    const Camera* camera = Root::Instance().GetCamera();
    const glm::vec3 normal = glm::normalize(camera->Direction() * -1.f);
    for (auto& component : mComponents)
    {
        component->draw(mRenderer, normal);
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
