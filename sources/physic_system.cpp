#include "physic_system.hpp"

#include "transform_system.hpp"
#include "types.hpp"
#include "game_entity.hpp"
#include "imgui/imgui_header.hpp"

#include "visualdebug.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <cassert>
#include <algorithm>


#if GUI_DEBUG()
void PhysicSystem::debug_GUI() const
{
    for (size_t idx = 0; idx < mComponents.size(); ++idx)
    {
        ImGui::PushID(idx);
        std::vector<PhysicComponent::ContactManifold>& contacts = const_cast<PhysicComponent&>(mComponents[idx]).mContacts;
        for (int ci = (int)contacts.size() - 1; 0 <= ci; --ci)
        {
            ImGui::PushID(ci);
            if (ImGui::SmallButton("Destroy"))
            {
                std::swap(contacts[ci], contacts[contacts.size() - 1]);
                contacts.resize(contacts.size() - 1);
            }
            else
            {
                PhysicComponent::ContactManifold& contact = contacts[ci];
                if (ImGui::IsItemHovered())
                {
                    VisualDebugSphereCommand sphere(contact.position, 0.1f, { 0, 1, 0, 1 });
                    VisualDebug()->PushCommand(sphere);
                }
                ImGui::InputFloat("distance", &contact.distance);
                ImGui::InputFloat3("normal", glm::value_ptr(contact.normal));
                ImGui::InputFloat("acc normal impulse", &contact.Pn);
                ImGui::InputFloat("acc tangent impulse", &contact.Pt);
                ImGui::InputFloat("mass normal", &contact.massNormal);
                ImGui::InputFloat("mass tangent", &contact.massTangent);
                ImGui::InputFloat("bias", &contact.bias);
            }
            ImGui::PopID();
            ImGui::Separator();
        }
        ImGui::PopID();
    }
}
#endif

PhysicComponent::ContactManifold::ContactManifold(const glm::vec3& a, const glm::vec3& b)
: position(b)
{
    const glm::vec3 diff = a - b;
    distance = glm::length(diff);
    normal = diff / distance;
}

PhysicComponent::PhysicComponent()
: mTransformComponent(nullptr)
, mForceAccum(0)
, mLinearVelocity(0,0,0,0)
, mLinearAcceleration(0,0,0,1)
, mAngularVelocity(0,0,0,0)
{}

PhysicComponent::PhysicComponent(const PhysicComponent& ref)
: mTransformComponent(ref.mTransformComponent)
, mEntity(ref.mEntity)
, mInvMass(ref.mInvMass)
, mInvI(ref.mInvI)
, mRadius(ref.mRadius)
, mForceAccum(ref.mForceAccum)
, mLinearVelocity(ref.mLinearVelocity)
, mLinearAcceleration(ref.mLinearAcceleration)
{}

bool PhysicComponent::IsValid() const
{
    return nullptr != mTransformComponent;
}

bool PhysicComponent::HasFiniteMass() const
{
    return 0.f != mInvMass;
}

void PhysicComponent::SetMass(const float mass)
{
    assert(0 <= mass);
    if (0 == mass)
    {
        mInvMass = 0.f;
        mInvI = 0.f;
    }
    else
    {
        mInvMass = 1.f / mass;
        const float inertia = (2.f / 5.f) * (mRadius * mRadius);
        mInvI = 1.f / inertia;
    }
}

void PhysicComponent::SetRadius(const float radius)
{
    assert(0.0f <= radius);
    mRadius = radius;
    SetMass(mInvMass == 0.f ? 0.f : 1.f / mInvMass);
}

void PhysicComponent::Reset()
{
    mForceAccum = glm::vec3(0);
    mLinearVelocity = glm::vec4(0, 0, 0, 0);
    mLinearAcceleration = glm::vec4(0, 0, 0, 1);
    mAngularVelocity = glm::vec4(0, 0, 0, 0);
    mContacts.clear();
}

void PhysicComponent::ResolveContacts(const float deltaTime, const float invDeltaTime)
{
    const glm::vec3 position(mTransformComponent->mPosition);
    const float k_allowedPenetration = -0.05f;
    const float k_biasFactor = true ? 0.2f : 0.0f;
    auto skipContact = [&k_allowedPenetration](const ContactManifold& c) -> bool
    {
        return 0 <= (c.distance + k_allowedPenetration);
    };

    // Pre Step
    {
        for (size_t i = 0; i < mContacts.size(); ++i)
        {
            ContactManifold& c = mContacts[i];
            if (skipContact(c))
            {
                c.bias = 0.f;
                c.Pn = 0.f;
                c.Pt = 0.f;
                continue;
            }
            const glm::vec3 r = c.position - position;
            const glm::vec3 cNormal = c.normal;

            // Precompute normal mass, tangent mass, and bias.
            const float rn = glm::dot(r, cNormal);
            const float kNormal = mInvMass + mInvI * (glm::dot(r, r) - rn * rn);
            c.massNormal = 1.0f / kNormal;

            const glm::vec3 cTangeant = glm::normalize(fabsf(cNormal.x) > fabsf(cNormal.z) ? glm::vec3(-cNormal.y, cNormal.x, 0.0) : glm::vec3(0.0, -cNormal.z, cNormal.y));
            const float rt = glm::dot(r, cTangeant);
            const float kTangent = mInvMass + mInvI * (glm::dot(r, r) - rt * rt);
            c.massTangent = 1.0f / kTangent;

            c.bias = -k_biasFactor * invDeltaTime * std::min(0.0f, c.distance + k_allowedPenetration);
            
            const glm::vec3 P = c.Pn * cNormal + c.Pt * cTangeant;
            mLinearVelocity += glm::vec4(mInvMass * P, 0.f);
            mAngularVelocity += glm::vec4(mInvI * glm::cross(r, P), 0.f);
        }
    }

    // Perform iterations
    for (int i = 0; i < 10; ++i)
    {
        for (size_t i = 0; i < mContacts.size(); ++i)
        {
            ContactManifold& c = mContacts[i];
            if (skipContact(c))
            {
                continue;
            }
            const glm::vec3 r = c.position - position;
            const glm::vec3 cNormal = c.normal;
            const glm::vec3 P = c.Pn * cNormal;

            // Relative velocity at contact
            glm::vec3 dv = glm::vec3(mLinearVelocity) + glm::cross(glm::vec3(mAngularVelocity), r);

            // Compute normal impulse
            const float vn = glm::dot(dv, cNormal);

            float dPn = c.massNormal * (-vn + c.bias);
            // Clamp the accumulated impulse
            {
                const float Pn0 = c.Pn;
                c.Pn = glm::max(Pn0 + dPn, 0.0f);
                dPn = c.Pn - Pn0;
            }

            // Apply contact impulse
            const glm::vec3 Pn = dPn * cNormal;

            mLinearVelocity += glm::vec4(mInvMass * Pn, 0.f);
            mAngularVelocity += glm::vec4(mInvI * glm::cross(r, Pn), 0.f);

            // Relative velocity at contact
            dv = glm::vec3(mLinearVelocity) + glm::cross(glm::vec3(mAngularVelocity), r);

            const glm::vec3 tangent = glm::normalize(fabsf(cNormal.x) > fabsf(cNormal.z) ? glm::vec3(-cNormal.y, cNormal.x, 0.0) : glm::vec3(0.0, -cNormal.z, cNormal.y));
            const float vt = glm::dot(dv, tangent);
            float dPt = c.massTangent * (-vt);

            {
                const float friction = 0.5f;
                
                // Compute friction impulse
                float maxPt = friction * c.Pn;

                // Clamp friction
                float oldTangentImpulse = c.Pt;
                c.Pt = glm::clamp(oldTangentImpulse + dPt, -maxPt, maxPt);
                dPt = c.Pt - oldTangentImpulse;
            }

            // Apply contact impulse
            const glm::vec3 Pt = dPt * tangent;

            mLinearVelocity += glm::vec4(mInvMass * Pt, 0.f);
            mAngularVelocity += glm::vec4(mInvI * glm::cross(r, Pt), 0.f);
        }
    }
}

void PhysicComponent::Integrate(const float deltaTime)
{
    if (!IsValid() || !HasFiniteMass())
        return;

    const glm::mat4 previousTransform = mTransformComponent->Transform();
    assert(false == glm::any(glm::isnan(previousTransform[0])));
    assert(false == glm::any(glm::isnan(previousTransform[1])));
    assert(false == glm::any(glm::isnan(previousTransform[2])));
    assert(false == glm::any(glm::isnan(previousTransform[3])));

    const glm::vec4 force(mForceAccum, 0.f);
    mLinearAcceleration += force*mInvMass;
    assert(0.f == mLinearVelocity.w);
    mLinearVelocity += mLinearAcceleration*deltaTime;
    mLinearVelocity.w = 0.f;
    const glm::vec4 position = mTransformComponent->Position();
    assert(1.f == position.w);
    const glm::vec4 linearDisplacement = mLinearVelocity * deltaTime;
    const glm::vec4 nextPosition = position + linearDisplacement;
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

    if (!mContacts.empty())
    {
        const glm::mat4 previousTransformInv = glm::inverse(previousTransform);
        assert(false == glm::any(glm::isnan(previousTransformInv[0])));
        assert(false == glm::any(glm::isnan(previousTransformInv[1])));
        assert(false == glm::any(glm::isnan(previousTransformInv[2])));
        assert(false == glm::any(glm::isnan(previousTransformInv[3])));
        const glm::mat4 transformDiff = previousTransformInv * mTransformComponent->Transform();
        for (int idx = numeric_cast<int>(mContacts.size() - 1); 0 <= idx; --idx)
        {
            ContactManifold& c = mContacts[idx];
            const glm::vec3 contactDisplacement = glm::vec3(transformDiff * glm::vec4(c.position, 1.f)) - c.position;
            const float displacementProjection = glm::dot(c.normal, contactDisplacement);
            c.distance += displacementProjection;
            if (0.1f < fabsf(c.distance) || 0.01f < glm::dot(contactDisplacement, contactDisplacement))
            {
                const size_t lastIdx = mContacts.size() - 1;
                std::swap(mContacts[idx], mContacts[lastIdx]);
                mContacts.resize(lastIdx);
            }
        }
    }

}

void PhysicComponent::AddForce(const glm::vec3& force)
{
    mForceAccum += force;
}

const glm::vec4& PhysicComponent::LinearVelocity() const
{
    return mLinearVelocity;
}

void PhysicComponent::SetLinearVelocity(const glm::vec4& velocity)
{
    mLinearVelocity = velocity;
}

const glm::vec4 & PhysicComponent::AngularVelocity() const
{
    return mAngularVelocity;
}

void PhysicComponent::SetAngularVelocity(const glm::vec4 & velocity)
{
    mAngularVelocity = velocity;
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
        if (!ci.IsValid() || !ci.HasFiniteMass())
            continue;
        ci.AddForce(glm::vec3(0.f, -9.81f, 0.f));
        const float radius = ci.mRadius;
        const float radiusSq = radius * radius;
        const glm::vec4& ciPosition = ci.mTransformComponent->mPosition;
        glm::vec4 ciVelocity = ci.LinearVelocity() * 0.5f;
        for (size_t ydx = idx + 1; ydx < componentsSize; ++ydx)
        {
            const PhysicComponent& cy = mComponents[ydx];
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
        }
        ci.SetLinearVelocity(ciVelocity);
    }
    
    const float invDeltaTime = 1.f / deltaTime;
    for (auto& component : mComponents)
    {
        component.ResolveContacts(deltaTime, invDeltaTime);
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
