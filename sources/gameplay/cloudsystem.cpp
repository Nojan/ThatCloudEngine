#include "cloudsystem.hpp"

#include "../game_entity.hpp"

#include <cassert>

void CloudComponent::Update(const float deltaTime)
{
}

CloudSystem::CloudSystem()
{
    mComponents.reserve(GameEntity::Max);
}

void CloudSystem::Update(const float deltaTime)
{

}

void CloudSystem::attachEntity(GameEntity * entity)
{
    CloudComponent& component = IComponentSystem::attachComponent<CloudComponent>(entity, mComponents);
    component.mPower = 0.f;
}

void CloudSystem::detachEntity(GameEntity * entity)
{
    IComponentSystem::detachComponent<CloudComponent>(entity, mComponents);
}
