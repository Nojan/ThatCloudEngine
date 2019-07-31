#include "gridsystem.hpp"

#include "../transform_system.hpp"
#include "../game_entity.hpp"
#include "../visualdebug.hpp"

void GridCellComponent::Update(const float deltaTime)
{
    if(mIsFilled)
        return;
    const glm::vec3 center(mTransform->mPosition);

    const glm::vec3 offset[] = {
        glm::vec3(-1.f, 0.f, -1.f),
        glm::vec3(-1.f, 0.f, 1.f),
        glm::vec3(1.f, 0.f, 1.f),
        glm::vec3(1.f, 0.f, -1.f),
        glm::vec3(-1.f, 0.f, -1.f),
    };

    const float scale = 15.f;

    for (int i = 0; i < 4; ++i)
    {
        VisualDebugSegmentCommand command(center + offset[i] * scale, center + offset[i+1] * scale, {1.f, 1.f, 1.f, 1.f});
        VisualDebug()->PushCommand(command);
    }
    
}

BoundingBox3D GridCellComponent::GetBoundingBox() const
{
    BoundingBox3D bbox;
    const glm::vec3 center(mTransform->mPosition);

    const glm::vec3 offset[] = {
        glm::vec3(-1.f, -FLT_MAX, -1.f),
        glm::vec3(-1.f, 0.f, 1.f),
        glm::vec3(1.f, FLT_MAX, 1.f),
        glm::vec3(1.f, 0.f, -1.f),
    };
    
    for (int i = 0; i < 4; ++i)
    {
        bbox.Add(center + offset[i] * GetSize());
    }
    
    return bbox;
}

float GridCellComponent::GetSize()
{
    return 15.0f;
}

GridCellSystem::GridCellSystem()
{
    mComponents.reserve(GameEntity::Max);
}

void GridCellSystem::Update(const float deltaTime)
{
    for (auto& component : mComponents)
    {
        component.Update(deltaTime);
    }
}

void GridCellSystem::attachEntity(GameEntity * entity)
{
    GridCellComponent& component = IComponentSystem::attachComponent<GridCellComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component.mTransform = tranform;
}

void GridCellSystem::detachEntity(GameEntity * entity)
{
    IComponentSystem::detachComponent<GridCellComponent>(entity, mComponents);
}

glm::ivec2 GridCellSystem::GetWorldPosition(const glm::ivec2 & gridPosition)
{
    const auto x = (-gridPosition.x - 7) * 30;
    const auto y = (-gridPosition.y + 57) * 30;
    return glm::ivec2(x, y);
}

float GridCellSystem::GetHeight()
{
    return 160.f;
}