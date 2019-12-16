#include "boy.hpp"

#include "gameconstant.hpp"

#include "../types.hpp"
#include "../global.hpp"
#include "../platform/platform.hpp"
#include "../game_entity.hpp"
#include "../game_system.hpp"
#include "../mesh_resource.hpp"

#include "../physic_system.hpp"
#include "../rendering_system.hpp"
#include "../renderableMesh.hpp"
#include "../resourcemesh.hpp"
#include "../resourcecache.hpp"
#include "../resourcemanager.hpp"
#include "../transform_system.hpp"

Boy::Boy()
{
    ResourceCache* cache = Global::resourceManager()->Cache();
    assert(cache);
    mResources.push_back(cache->get_or_create<ResourceMesh>("cloudboy_slow"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("cloudboy_normal"));
    mResources.push_back(cache->get_or_create<ResourceMesh>("cloudboy_fast"));
}

void Boy::ListResources(std::vector<Resource*>& resources)
{
    resources.reserve(resources.size() + mResources.size());
    for (auto& r : mResources)
    {
        resources.push_back(r.get());
    }
}

void Boy::OnLoad()
{
}

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

    renderingComponent->setupResource(mResources.front()->Mesh());
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
    const float diff_len = glm::length(diff);
    const glm::vec3 direction = diff_len == 0.f ? glm::vec3(0,0,0) : diff / diff_len;
    const float speed = glm::min(diff_len * 0.1f / deltaTime, Gameplay::max_flying_speed);
    const glm::vec4 v(direction * speed, 0.f);
    physic->SetLinearVelocity(v);

    // Set model
    {
        int idx = mResources.size() - 1;
        if (speed < 5.f)
        {
            idx = 0;
        }
        else if (speed < 10.f)
        {
            idx = 1;
        }
        GraphicMeshComponent* renderingComponent = mEntity->getComponent<GraphicMeshComponent>();
        renderingComponent->setupResource(mResources[idx]->Mesh());
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
