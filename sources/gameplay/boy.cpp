#include "boy.hpp"

#include "../types.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"

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

    gameSystem->getSystem<RenderingSystem>()->attachEntity(entity);
    GraphicMeshComponent* renderingComponent = entity->getComponent<GraphicMeshComponent>();
    renderingComponent->mColor = { 0.f, 0.f, 1.f, 1.f };

    char filename[256];
    sprintf(filename, "../assets/3D/%s.assxml", "cloudboy_normal");
    renderingComponent->setupResource( Global::resourceManager()->meshResource(filename) );
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
