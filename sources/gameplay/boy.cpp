#include "boy.hpp"

#include "../types.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"

#include "../physic_system.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../resourcemanager.hpp"
#include "../transform_system.hpp"

#include "../tinyxml/tinyxml2.h"

void Boy::Init()
{
    GameSystem* gameSystem = Global::gameSytem();
    GameEntity* entity = gameSystem->createEntity();
    mEntity.reset(entity);

    gameSystem->getSystem<TransformSystem>()->attachEntity(entity);
    TransformComponent* transform = entity->getComponent<TransformComponent>();

    gameSystem->getSystem<PhysicSystem>()->attachEntity(entity);

    gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
    GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
    renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };

    char filename[256];
    sprintf(filename, "../assets/3D/%s.assxml", "cloudboy_slow");
    mMeshResourceList.push_back(Global::resourceManager()->meshResource(filename));
    sprintf(filename, "../assets/3D/%s.assxml", "cloudboy_normal");
    mMeshResourceList.push_back(Global::resourceManager()->meshResource(filename));
    sprintf(filename, "../assets/3D/%s.assxml", "cloudboy_fast");
    mMeshResourceList.push_back(Global::resourceManager()->meshResource(filename));
    renderingComponent->setupResource( mMeshResourceList.front() );
}

void Boy::Terminate()
{
    if (mEntity)
    {
        GameSystem* gameSystem = Global::gameSytem();
        gameSystem->removeEntity(mEntity.release());
    }
}

void Boy::TeleportTo(const glm::vec3 & position)
{
    TransformComponent* transform = mEntity->getComponent<TransformComponent>();
    transform->SetPosition(glm::vec4(position, 1.f));
}

void Boy::MoveToward(const glm::vec3 & position, const float deltaTime)
{
    if(deltaTime == 0.f)
        return;
    
    // Compute velocity
    PhysicComponent* physic = mEntity->getComponent<PhysicComponent>();
    const glm::vec3 currentPosition(physic->mTransformComponent->Position());
    const glm::vec3 diff = position - currentPosition;
    const glm::vec4 v(diff * 0.1f / deltaTime, 0.f);
    physic->SetLinearVelocity(v);

    // Set model
    {
        const float speed = glm::length(glm::vec3(v));
        int idx = mMeshResourceList.size() - 1;
        if (speed < 5.f)
        {
            idx = 0;
        }
        else if (speed < 10.f)
        {
            idx = 1;
        }
        GraphicMeshComponent* renderingComponent = mEntity->getComponent<GraphicMeshComponent>();
        renderingComponent->setupResource(mMeshResourceList[idx]);
    }   

    // Update rotation
    const float diffLength = glm::length(diff);
    if(diffLength <= 1.f)
        return;
    const glm::vec3 forward = diff / diffLength;

    const glm::quat r(glm::vec3(0.f, 0.f, -1.f), forward);
    physic->mTransformComponent->SetRotation(r);
}

glm::vec3 Boy::Position() const
{
    PhysicComponent* physic = mEntity->getComponent<PhysicComponent>();
    const glm::vec3 position(physic->mTransformComponent->Position());
    return position;
}
