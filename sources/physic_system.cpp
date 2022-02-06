#include "physic_system.hpp"

#include "transform_system.hpp"
#include "types.hpp"
#include "game_entity.hpp"
#include "imgui/imgui_header.hpp"

#include "visualdebug.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/epsilon.hpp>
#include <cassert>
#include <algorithm>

namespace Constant {
    IMGUI_VAR(DrawContacts, false);
}

#if GUI_DEBUG()
void PhysicSystem::debug_GUI() const
{
    ImGui::Checkbox("DrawContacts", &Constant::DrawContacts);
    std::vector<PhysicComponent::ContactManifold>& contacts = const_cast<std::vector<PhysicComponent::ContactManifold>&>(mContactsCache);
    if (ImGui::CollapsingHeader("Contact cache"))
    {
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
    }
}
#endif

PhysicComponent::ContactManifold::ContactManifold(const glm::vec3& a, const glm::vec3& b, PhysicComponent* bodyA, PhysicComponent* bodyB)
: bodyA(bodyA)
, bodyB(bodyB)
, position(b)
{
    const glm::vec3 diff = a - b;
    distance = glm::length(diff);
    normal = diff / distance;
    tangeant = glm::normalize(fabsf(normal.x) > fabsf(normal.z) ? glm::vec3(-normal.y, normal.x, 0.0) : glm::vec3(0.0, -normal.z, normal.y));
    auto computeLocalPosition = [](const glm::mat4& transform, const glm::vec3& point) -> glm::vec3
    {
        const glm::mat4 transformInv = glm::inverse(transform);
        assert(false == glm::any(glm::isnan(transformInv[0])));
        assert(false == glm::any(glm::isnan(transformInv[1])));
        assert(false == glm::any(glm::isnan(transformInv[2])));
        assert(false == glm::any(glm::isnan(transformInv[3])));
        return glm::vec3(transformInv * glm::vec4(point, 1.f));
    };
    if (bodyA)
    {
        localPositionA = computeLocalPosition(bodyA->mTransformComponent->Transform(), a);
    }
    if (bodyB)
    {
        localPositionB = computeLocalPosition(bodyB->mTransformComponent->Transform(), b);
    }
}

PhysicComponent::ContactManifold::ContactManifold(const float distance, const glm::vec3& normal, const glm::vec3& b, PhysicComponent* bodyA, PhysicComponent* bodyB)
: bodyA(bodyA)
, bodyB(bodyB)
, position(b)
, normal(normal)
, distance(distance)
{
    tangeant = glm::normalize(fabsf(normal.x) > fabsf(normal.z) ? glm::vec3(-normal.y, normal.x, 0.0) : glm::vec3(0.0, -normal.z, normal.y));
    auto computeLocalPosition = [](const glm::mat4& transform, const glm::vec3& point) -> glm::vec3
    {
        const glm::mat4 transformInv = glm::inverse(transform);
        assert(false == glm::any(glm::isnan(transformInv[0])));
        assert(false == glm::any(glm::isnan(transformInv[1])));
        assert(false == glm::any(glm::isnan(transformInv[2])));
        assert(false == glm::any(glm::isnan(transformInv[3])));
        return glm::vec3(transformInv * glm::vec4(point, 1.f));
    };
    if (bodyA)
    {
        const glm::vec3 a = b + normal * distance;
        localPositionA = computeLocalPosition(bodyA->mTransformComponent->Transform(), a);
    }
    if (bodyB)
    {
        localPositionB = computeLocalPosition(bodyB->mTransformComponent->Transform(), b);
    }
}

PhysicComponent::PhysicComponent()
{
    Initialize();
}

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

PhysicComponent::~PhysicComponent()
{
    Invalidate();
}

void PhysicComponent::Initialize()
{
    mTransformComponent = nullptr;
    mRadius = 0.f;
    Reset();
}

void PhysicComponent::Invalidate()
{
    mRadius = std::numeric_limits<float>::quiet_NaN();
    mTransformComponent = nullptr;
}

bool PhysicComponent::IsValid() const
{
    return !std::isnan(mRadius);
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

const glm::mat4& PhysicComponent::GetTransform() const
{
    return mTransform;
}

const glm::mat4& PhysicComponent::GetTransformInv() const
{
    return mTransformInv;
}

bool PhysicComponent::UpdateTransform()
{
    assert(mTransformComponent);
    const glm::mat4 currentTransform = mTransformComponent->Transform();
    bool updateRequired = false;
    for (int idx = 0; idx < 4; ++idx)
    {
        assert(false == glm::any(glm::isnan(currentTransform[idx])));
        updateRequired = updateRequired || glm::any(glm::epsilonNotEqual(currentTransform[idx], mTransform[idx], 0.0001f));
    }
    if (updateRequired)
    {
        mTransform = currentTransform;
        mTransformInv = glm::inverse(mTransform);
        for (int idx = 0; idx < 4; ++idx)
        {
            assert(false == glm::any(glm::isnan(mTransformInv[idx])));
        }
    }
    return updateRequired;
}

void PhysicComponent::Reset()
{
    mForceAccum = glm::vec3(0);
    mLinearVelocity = glm::vec4(0, 0, 0, 0);
    mLinearAcceleration = glm::vec4(0, 0, 0, 1);
    mAngularVelocity = glm::vec4(0, 0, 0, 0);
    mContactIdx.clear();
}

void PhysicComponent::Integrate(const float deltaTime)
{
    if (!IsValid() || !HasFiniteMass())
        return;

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

    UpdateTransform();

    //Reset
    mForceAccum = glm::vec3(0.f);
    mLinearAcceleration = glm::vec4(0.f);
    //Drag
    const glm::vec4 drag(0.9999f);
    mLinearVelocity = mLinearVelocity * drag;
    mAngularVelocity = mAngularVelocity  * drag;

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
        if (ci.UpdateTransform())
        {
            ClearContactsCache(&ci);
        }
        ci.AddForce(glm::vec3(0.f, -9.81f, 0.f));
#if 0
        glm::vec4 ciVelocity = ci.LinearVelocity() * 0.5f;
        ci.SetLinearVelocity(ciVelocity);
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
#endif
    }
    
    const float invDeltaTime = 1.f / deltaTime;

    // resolve contacts
    {
        const float k_allowedPenetration = -0.05f;
        const float k_biasFactor = true ? 0.2f : 0.0f;
        auto skipContact = [&k_allowedPenetration](const PhysicComponent::ContactManifold& c) -> bool
        {
            return 0 <= (c.distance + k_allowedPenetration) || nullptr == c.bodyA || nullptr == c.bodyB;
        };

        auto applyContactSeparation = [](PhysicComponent* body, const glm::vec3& p, const glm::vec3& r, const float s) -> void
        {
            glm::vec3 linearVelocity(body->LinearVelocity());
            glm::vec3 angularVelocity(body->AngularVelocity());

            linearVelocity += body->mInvMass * p * s;
            angularVelocity += body->mInvI * glm::cross(r, p) * s;

            body->SetLinearVelocity(glm::vec4(linearVelocity, 0.f));
            body->SetAngularVelocity(glm::vec4(angularVelocity, 0.f));
        };

        auto relativeVelocity = [](PhysicComponent* body, const glm::vec3& r) -> glm::vec3
        {
            return glm::vec3(body->LinearVelocity()) + glm::cross(glm::vec3(body->AngularVelocity()), r);
        };

        constexpr float signA = 1.f;
        constexpr float signB = -1.f;

        // Pre Step
        for (size_t i = 0; i < mContactsCache.size(); ++i)
        {
            PhysicComponent::ContactManifold& c = mContactsCache[i];
            if (skipContact(c))
            {
                c.bias = 0.f;
                c.Pn = 0.f;
                c.Pt = 0.f;
                continue;
            }

            const glm::vec3 positionA(c.bodyA->GetTransform()[3]);
            const glm::vec3 positionB(c.bodyB->GetTransform()[3]);

            const glm::vec3 rA = c.position - positionA;
            const glm::vec3 rB = c.position - positionB;
            const glm::vec3 cNormal = c.normal;

            // Precompute normal mass, tangent mass, and bias.
            const float rnA = glm::dot(rA, cNormal);
            const float rnB = glm::dot(rB, cNormal);
            const float kNormal = 
              c.bodyA->mInvMass + c.bodyA->mInvI * (glm::dot(rA, rA) - rnA * rnA)
            + c.bodyB->mInvMass + c.bodyB->mInvI * (glm::dot(rB, rB) - rnB * rnB);
            c.massNormal = 1.0f / kNormal;

            const glm::vec3 cTangeant = c.tangeant;
            const float rtA = glm::dot(rA, cTangeant);
            const float rtB = glm::dot(rB, cTangeant);
            const float kTangent = 
              c.bodyA->mInvMass + c.bodyA->mInvI * (glm::dot(rA, rA) - rtA * rtA)
            + c.bodyB->mInvMass + c.bodyB->mInvI * (glm::dot(rB, rB) - rtB * rtB);
            c.massTangent = 1.0f / kTangent;

            c.bias = -k_biasFactor * invDeltaTime * std::min(0.0f, c.distance + k_allowedPenetration);

            const glm::vec3 P = c.Pn * cNormal + c.Pt * cTangeant;

            applyContactSeparation(c.bodyA, P, rA, signA);
            applyContactSeparation(c.bodyB, P, rB, signB);
        }

        // Perform iterations
        for (int i = 0; i < 10; ++i)
        {
            for (size_t i = 0; i < mContactsCache.size(); ++i)
            {
                PhysicComponent::ContactManifold& c = mContactsCache[i];
                if (skipContact(c))
                {
                    continue;
                }
                const glm::vec3 positionA(c.bodyA->GetTransform()[3]);
                const glm::vec3 positionB(c.bodyB->GetTransform()[3]);

                const glm::vec3 rA = c.position - positionA;
                const glm::vec3 rB = c.position - positionB;

                const glm::vec3 cNormal = c.normal;
                const glm::vec3 P = c.Pn * cNormal;

                // Relative velocity at contact
                glm::vec3 dv = relativeVelocity(c.bodyA, rA) - relativeVelocity(c.bodyB, rB);

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

                applyContactSeparation(c.bodyA, Pn, rA, signA);
                applyContactSeparation(c.bodyB, Pn, rB, signB);

                // friction x
                {
                    // Relative velocity at contact
                    dv = relativeVelocity(c.bodyA, rA) - relativeVelocity(c.bodyB, rB);

                    const glm::vec3 tangent = c.tangeant;
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

                    applyContactSeparation(c.bodyA, Pt, rA, signA);
                    applyContactSeparation(c.bodyB, Pt, rB, signB);
                }

                // friction y
                if(false)
                {
                    // Relative velocity at contact
                    dv = relativeVelocity(c.bodyA, rA) - relativeVelocity(c.bodyB, rB);

                    const glm::vec3 tangent = glm::cross(c.normal, c.tangeant);
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

                    applyContactSeparation(c.bodyA, Pt, rA, signA);
                    applyContactSeparation(c.bodyB, Pt, rB, signB);
                }
            }
        }
    }


    for (auto& component : mComponents)
    {
        component.Integrate(deltaTime);
    }

    VisualDebugRenderer* visualDebug = VisualDebug();
    auto drawContactManifold = [visualDebug](const PhysicComponent::ContactManifold& contact)
    {
        const Color::rgbap color = { 1, 1, 1, 1 };
        visualDebug->PushCommand(VisualDebugSegmentCommand(contact.position, contact.position + contact.normal, color));
        visualDebug->PushCommand(VisualDebugHalfCone(contact.position + (contact.normal * 0.8f), contact.position + contact.normal, 0.1f, 0.f, color));
    };

    for (int idx = numeric_cast<int>(mContactsCache.size()) - 1; 0 <= idx; --idx)
    {
        PhysicComponent::ContactManifold& c = mContactsCache[idx];

        const glm::mat4 transformA = c.bodyA ? c.bodyA->GetTransform() : glm::mat4(1.f);
        const glm::mat4 transformB = c.bodyB ? c.bodyB->GetTransform() : glm::mat4(1.f);

        const glm::vec3 positionA = glm::vec3(transformA * glm::vec4(c.localPositionA, 1.f));
        const glm::vec3 positionB = glm::vec3(transformB * glm::vec4(c.localPositionB, 1.f));

        const glm::vec3 diff = positionA - positionB;
        const glm::vec3 diffNormalize = glm::normalize(diff);
        if (glm::dot(diffNormalize, c.normal) < 0.9f) // revoir ce critere. Quand la distance entre les points est courte, la normal n'est si importante.
        {
            RemoveContact(idx);
        }
        else
        {
            c.distance = glm::dot(c.normal, diff);
            if (Constant::DrawContacts)
                drawContactManifold(c);
        }
    }
}

int PhysicSystem::CreateContact(const PhysicComponent::ContactManifold& contact)
{
    auto findBestMatch = [](const PhysicComponent::ContactManifold & contact, const std::vector<PhysicComponent::ContactManifold> & collections) -> int
    {
        int result = -1;
        float bestMatch = 0.01f;
        auto computeMatch = [](const PhysicComponent::ContactManifold& a, const PhysicComponent::ContactManifold& b) -> float
        {
            float result = FLT_MAX;
            if (a.bodyA == b.bodyA && a.bodyB == b.bodyB)
            {
                const glm::vec3 diff = a.position - b.position;
                result = glm::dot(diff, diff); // + fabsf(1.f - glm::dot(a.normal, b.normal));
            }
            return result;
        };
        for (int idx = 0, endIdx = numeric_cast<int>(collections.size()); idx < endIdx; ++idx)
        {
            const PhysicComponent::ContactManifold& candidate = collections[idx];
            const float match = computeMatch(contact, candidate);
            if (match < bestMatch)
            {
                bestMatch = match;
                result = idx;
            }
        }
        return result;
    };

    int result = findBestMatch(contact, mContactsCache);
    if (0 <= result)
    {
        mContactsCache[result] = contact;
    }
    else
    {
        auto findContacts = [](const PhysicComponent::ContactManifold& contact, const std::vector<PhysicComponent::ContactManifold>& collections, std::vector<int>& found) -> void
        {
            for (int idx = 0; idx < collections.size(); ++idx )
            {
                const PhysicComponent::ContactManifold& c = collections[idx];
                if (c.bodyA == contact.bodyA && c.bodyB == contact.bodyB)
                {
                    found.push_back(idx);
                }
            }
        };
        auto sortContact = [&contactsCache = mContactsCache](const int a, const int b) -> bool
        {
            return contactsCache[a].distance < contactsCache[b].distance;
        };
        auto triangleAreaEstimate = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) -> float
        {
            const glm::vec3 ab = b - a;
            const glm::vec3 ac = c - a;
            const glm::vec3 cross = glm::cross(ab, ac);
            const float parallelogrameAreaSquared = glm::dot(cross, cross);
            return parallelogrameAreaSquared;
        };
        std::vector<int> contacts;
        contacts.reserve(4);
        findContacts(contact, mContactsCache, contacts);
        if (3 < contacts.size())
        {
            std::sort(contacts.begin(), contacts.end(), sortContact);
            const float currentArea = triangleAreaEstimate(mContactsCache[contacts[1]].position, mContactsCache[contacts[2]].position, mContactsCache[contacts[3]].position);
            int bestIdx = -1;
            for (int cIdx = 1; cIdx <= 3; ++cIdx)
            {
                const float area = triangleAreaEstimate(1 == cIdx ? contact.position : mContactsCache[contacts[1]].position, 2 == cIdx ? contact.position : mContactsCache[contacts[2]].position, 3 == cIdx ? contact.position : mContactsCache[contacts[3]].position);
                if (currentArea < area)
                {
                    bestIdx = cIdx;
                }
                if (0 < bestIdx)
                {
                    result = contacts[bestIdx];
                    PhysicComponent::ContactManifold& mergedContact = mContactsCache[result];
                    constexpr bool warmup = true;
                    const float Pn = mergedContact.Pn;
                    const float Pt = mergedContact.Pt;
                    mergedContact = contact;
                    if (warmup)
                    {
                        mergedContact.Pn = Pn;
                        mergedContact.Pt = Pt;
                    }
                }
            }
        }
        else
        {
            for (int idx = 0, endIdx = numeric_cast<int>(mContactsCache.size()); idx < endIdx; ++idx)
            {
                PhysicComponent::ContactManifold& candidate = mContactsCache[idx];
                if (nullptr == candidate.bodyA && nullptr == candidate.bodyB)
                {
                    candidate = contact;
                    result = idx;
                    break;
                }
            }
            if (result < 0)
            {
                mContactsCache.push_back(contact);
                result = numeric_cast<int>(mContactsCache.size()) - 1;
            }
            if (contact.bodyA)
            {
                contact.bodyA->mContactIdx.push_back(result);
            }
            if (contact.bodyB)
            {
                contact.bodyB->mContactIdx.push_back(result);
            }
        }
    }
    return result;
}

void PhysicSystem::RemoveContact(int idx)
{
    assert(0 <= idx);
    assert(idx < numeric_cast<int>(mContactsCache.size()));
    auto removeFromComponent = [](PhysicComponent* component, int contactIdx) -> void
    {
        if (component)
        {
            std::vector<int>& contactsCacheIdx = component->mContactIdx;
            for (int idx = numeric_cast<int>(contactsCacheIdx.size()) - 1; 0 <= idx; --idx)
            {
                if (contactIdx == contactsCacheIdx[idx])
                {
                    const size_t lastIdx = contactsCacheIdx.size() - 1;
                    std::swap(contactsCacheIdx[idx], contactsCacheIdx[lastIdx]);
                    contactsCacheIdx.resize(lastIdx);
                }
            }
        }
    };
    PhysicComponent::ContactManifold& contact = mContactsCache[idx];
    removeFromComponent(contact.bodyA, idx);
    removeFromComponent(contact.bodyB, idx);
    contact = PhysicComponent::ContactManifold();
}

void PhysicSystem::ClearContactsCache(PhysicComponent* component)
{
    std::vector<int>& contactsCacheIdx = component->mContactIdx;
    while(!contactsCacheIdx.empty())
    {
        RemoveContact(contactsCacheIdx.front());
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
    if(PhysicComponent* component = entity->getComponent<PhysicComponent>())
    {
        ClearContactsCache(component);
    }
    IComponentSystem::detachComponent<PhysicComponent>(entity, mComponents);
}
