#include "physic_system.hpp"

#include "transform_system.hpp"
#include "game_entity.hpp"
#include "visualdebug.hpp"
#include "imgui/imgui_header.hpp"

#include <glm/gtx/norm.hpp>
#include <cassert>

constexpr float awakeCounter = 5.f;

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mEntity(nullptr)
, mInvMass(1)
, mForceAccum(0)
, mLinearVelocity(0,0,0,0)
, mLinearAcceleration(0,0,0,1)
, mAngularVelocity(0,0,0,0)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mEntity(ref.mEntity)
, mInvMass(ref.mInvMass)
, mRadius(ref.mRadius)
, mForceAccum(ref.mForceAccum)
, mLinearVelocity(ref.mLinearVelocity)
, mLinearAcceleration(ref.mLinearAcceleration)
{}

bool PhysicComponent::IsValid() const
{
    return nullptr != mTransformComponent;
}

bool PhysicComponent::IsAsleep() const
{
    return 0.f >= mAwake || awakeCounter < mAwake;
}

bool PhysicComponent::HasFiniteMass() const
{
    return 0.f != mInvMass;
}

void PhysicComponent::SetMass(const float mass)
{
    assert(0 <= mass);
    if (0 == mass)
        mInvMass = FLT_MAX;
    else
        mInvMass = 1.f / mass;
}

void PhysicComponent::SetRadius(const float radius)
{
    assert(0.0f <= radius);
    mRadius = radius;
}

void PhysicComponent::Reset()
{
    mForceAccum = glm::vec3(0);
    mLinearVelocity = glm::vec4(0, 0, 0, 0);
    mLinearAcceleration = glm::vec4(0, 0, 0, 1);
    mAngularVelocity = glm::vec4(0, 0, 0, 0);
    mAwake = 0.f;
}

void PhysicComponent::Integrate(const float deltaTime)
{
    mAwake -= deltaTime;
    
    if (!IsValid() || !HasFiniteMass() || IsAsleep())
        return;

    const glm::vec4 force(mForceAccum, 0.f);
    mLinearAcceleration += force*mInvMass;
    assert(0.f == mLinearVelocity.w);
    mLinearVelocity += mLinearAcceleration*deltaTime;
    mLinearVelocity.w = 0.f;
    const glm::vec4 position = mTransformComponent->Position();
    assert(1.f == position.w);
    const glm::vec4 nextPosition = position + mLinearVelocity*deltaTime;
    mTransformComponent->SetPosition(nextPosition);

    const glm::quat& currentOrientation = mTransformComponent->Rotation();
    const glm::quat angularVelocityQuat(0, mAngularVelocity.x, mAngularVelocity.y, mAngularVelocity.z);
    const glm::quat spin = deltaTime * 0.5f * angularVelocityQuat * currentOrientation;
    const glm::quat newOrientation = currentOrientation + spin;
    mTransformComponent->SetRotation(glm::normalize(newOrientation));

    //Reset
    mForceAccum = glm::vec3(0.f);
    mLinearAcceleration = glm::vec4(0.f);
    //Drag
    const glm::vec4 drag(0.9999f);
    mLinearVelocity = mLinearVelocity * drag;
    mAngularVelocity = mAngularVelocity  * drag;

    if (0.01f < glm::dot(mLinearVelocity, mLinearVelocity) || 0.01f < glm::dot(mAngularVelocity, mAngularVelocity))
    {
        SetAwake();
    }
}

void PhysicComponent::AddForce(const glm::vec3& force)
{
    mForceAccum += force;
    SetAwake();
}

const glm::vec4& PhysicComponent::LinearVelocity() const
{
    return mLinearVelocity;
}

void PhysicComponent::SetLinearVelocity(const glm::vec4& velocity)
{
    mLinearVelocity = velocity;
    if(0.01f < glm::length2(velocity))
        SetAwake();
}

const glm::vec4 & PhysicComponent::AngularVelocity() const
{
    return mAngularVelocity;
}

void PhysicComponent::SetAngularVelocity(const glm::vec4& velocity)
{
    mAngularVelocity = velocity;
    if(0.01f < glm::length2(velocity))
        SetAwake();
}

void PhysicComponent::SetAwake()
{
    mAwake = awakeCounter;
}

PhysicSystem::PhysicSystem()
{
    mComponents.reserve(GameEntity::Max);
}

PhysicSystem::~PhysicSystem()
{}

void PhysicSystem::Update(const float deltaTime)
{
    assert(0 <= deltaTime);
    const size_t componentsSize = mComponents.size();
    for (size_t idx = 0; idx < componentsSize; ++idx)
    {
        PhysicComponent& ci = mComponents[idx];
        if (!ci.IsValid() || !ci.HasFiniteMass() || ci.IsAsleep())
            continue;
        const float radius = ci.mRadius;
        const float radiusSq = radius * radius;
        const glm::vec4& ciPosition = ci.mTransformComponent->mPosition;
        glm::vec4 ciVelocity = ci.LinearVelocity() * 0.5f;
        for (size_t ydx = idx + 1; ydx < componentsSize; ++ydx)
        {
            PhysicComponent& cy = mComponents[ydx];
            if (!cy.IsValid())
                continue;
            const glm::vec4& cyPosition = cy.mTransformComponent->mPosition;
            const glm::vec4 diffP = ciPosition - cyPosition;
            const float diffMagSq = glm::dot(diffP, diffP);
            if( 0.f == diffMagSq)
                continue; // superposition
            const float penetrationMagSq = diffMagSq - (4.f * radiusSq);
            if( 0.f < penetrationMagSq)
                continue; // no penetration
            const float penetrationMag = sqrt(-penetrationMagSq);
            const float diffMag = sqrt(diffMagSq);
            const glm::vec4 diffNormal = diffP / diffMag;
            ciVelocity += diffNormal * penetrationMag;
            if (m_listener)
            {
                PhysicEvent e = {ci.mEntity, cy.mEntity, &ciVelocity};
                m_listener->OnPhysicsEvent(e);
            }
            cy.SetAwake();
        }
        ci.SetLinearVelocity(ciVelocity);
    }
    
    for (auto& component : mComponents)
    {
        component.Integrate(deltaTime);
    }
}

void PhysicSystem::attachEntity(GameEntity* entity)
{
    PhysicComponent& component = IComponentSystem::attachComponent<PhysicComponent>(entity, mComponents);
    TransformComponent* tranform = entity->getComponent<TransformComponent>();
    assert(tranform);
    component.mTransformComponent = tranform;
    component.mEntity = entity;
}

void PhysicSystem::detachEntity(GameEntity* entity)
{
    IComponentSystem::detachComponent<PhysicComponent>(entity, mComponents);
}

#ifdef IMGUI_ENABLE
void PhysicSystem::debug_GUI() const
{
    static bool displayAabb = true;
    ImGui::Checkbox("Display AABB", &displayAabb);
    if(displayAabb)
    {
        VisualDebugRenderer * renderer = VisualDebug();
        for (const auto& component : mComponents)
        {
            if(!component.IsValid())
            {
                continue;
            }
            const float radius = component.mRadius;
            const glm::vec3 position(component.mTransformComponent->Position());
            constexpr float alpha = 0.2f;
            Color::rgbap color = {1.f, 1.f, 1.f, alpha};
            {
                if(!component.HasFiniteMass())
                {
                    color  = {0.f, 0.f, 0.f, alpha};
                }
                else if(!component.IsAsleep())
                {
                    color  = {0.f, 1.f, 0.f, alpha};
                }
            }
            VisualDebugCubeCommand command(position, radius, color);
            renderer->PushCommand(command);
        }
    }
}
#endif
