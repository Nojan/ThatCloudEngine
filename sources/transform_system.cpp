#include "transform_system.hpp"

#include "game_entity.hpp"

#include <cassert>

TransformComponent::TransformComponent()
{
    Initialize();
}

TransformComponent::TransformComponent(const TransformComponent& ref)
: mPosition(ref.mPosition)
, mScale(ref.mScale)
, mRotation(ref.mRotation)
{
}

void TransformComponent::Initialize()
{
    mPosition = glm::vec4(0.f, 0.f, 0.f, 0.f);
    mScale = glm::mat4(1.f);
    mRotation = glm::quat(1, 0, 0, 0);
}

void TransformComponent::Invalidate()
{
    mPosition = glm::vec4(0.f, 0.f, 0.f, std::numeric_limits<float>::quiet_NaN());
    mScale = glm::mat4(1.f);
    mRotation = glm::quat(1, 0, 0, 0);
}

const glm::vec4& TransformComponent::Position() const
{
    return mPosition;
}

bool TransformComponent::Invalid() const
{
    return std::isnan(mPosition.w);
}

void TransformComponent::SetPosition(const glm::vec4& position)
{
    mPosition = position;
}

const glm::quat& TransformComponent::Rotation() const
{
    return mRotation;
}

void TransformComponent::SetRotation(const glm::quat& rotation)
{
    mRotation = rotation;
}

glm::mat4 TransformComponent::Transform() const
{
    assert(!Invalid());
    glm::mat4 result;
    result = glm::mat4_cast(mRotation);
    result[3] = Position();
    return result;
}

TransformSystem::TransformSystem()
{
    mComponents.reserve(GameEntity::Max);
}

TransformSystem::~TransformSystem()
{}

void TransformSystem::Update(const float deltaTime)
{ }

void TransformSystem::attachEntity(GameEntity* entity) 
{
    TransformComponent& component = IComponentSystem::attachComponent<TransformComponent>(entity, mComponents);
    component.mPosition = glm::vec4(0.f, 0.f, 0.f, 1.f);
}

void TransformSystem::detachEntity(GameEntity* entity) 
{
    IComponentSystem::detachComponent<TransformComponent>(entity, mComponents);
}
