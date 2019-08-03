#include "gridsystem.hpp"

#include "gameconstant.hpp"
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

    const float scale = GetSize();

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
    return Gameplay::grid_size;
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

    if(mShowGrid)
    {
        const Color::rgbap color = {0.21f, 0.35f, 0.49f, 1.f};
        const int xmin = -60;
        const int xmax = 60;
        const int ymin = 0;
        const int ymax = 115;
        for (int x = xmin; x < xmax; ++x)
        {
            const glm::ivec2 begin = GetWorldPosition(glm::ivec2(x, ymin));
            const glm::ivec2 end = GetWorldPosition(glm::ivec2(x, ymax));
            VisualDebugSegmentCommand command(glm::vec3(begin.x - GridCellComponent::GetSize(), GridCellSystem::GetHeight(), begin.y), glm::vec3(end.x - GridCellComponent::GetSize(), GridCellSystem::GetHeight(), end.y), color);
            VisualDebug()->PushCommand(command);
        }
        
        for (int y = ymin; y < ymax; ++y)
        {
            const glm::ivec2 begin = GetWorldPosition(glm::ivec2(xmin, y));
            const glm::ivec2 end = GetWorldPosition(glm::ivec2(xmax, y));
            VisualDebugSegmentCommand command(glm::vec3(begin.x, GridCellSystem::GetHeight(), begin.y - GridCellComponent::GetSize()), glm::vec3(end.x, GridCellSystem::GetHeight(), end.y - GridCellComponent::GetSize()), color);
            VisualDebug()->PushCommand(command);
        }
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

void GridCellSystem::ShowDebugGrid(bool debug)
{
    mShowGrid = debug;
}

glm::ivec2 GridCellSystem::GetWorldPosition(const glm::ivec2 & gridPosition)
{
    const auto x = (-gridPosition.x - 7) * 30;
    const auto y = (-gridPosition.y + 57) * 30;
    return glm::ivec2(x, y);
}

float GridCellSystem::GetHeight()
{
    return Gameplay::grid_altitude;
}