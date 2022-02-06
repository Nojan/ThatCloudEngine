#pragma once

#include "icomponentsystem.hpp"
#include "physics_event.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class TransformComponent;

class PhysicComponent
{
public:
    struct ContactManifold {
        ContactManifold() = default;
        ContactManifold(const glm::vec3& a, const glm::vec3& b, PhysicComponent* bodyA, PhysicComponent* bodyB);
        ContactManifold(const float distance, const glm::vec3& normal, const glm::vec3& b, PhysicComponent* bodyA, PhysicComponent* bodyB);

        PhysicComponent* bodyA = nullptr;
        PhysicComponent* bodyB = nullptr;
        glm::vec3 localPositionA;
        glm::vec3 localPositionB;
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 tangeant;
        float distance = 0.f;
        float Pn = 0.f;	 // accumulated normal impulse
        float Pt = 0.f;	 // accumulated tangent impulse
        float massNormal = 0.f;
        float massTangent = 0.f;
        float bias = 0.f;
    };

    PhysicComponent();
    PhysicComponent(const PhysicComponent& ref);
    ~PhysicComponent();

    void Initialize();
    void Invalidate();

    bool IsValid() const;

    bool HasFiniteMass() const;
    void SetMass(const float mass);
    void SetRadius(const float radius);

    const glm::mat4& GetTransform() const;
    const glm::mat4& GetTransformInv() const;
    bool UpdateTransform();

    void Reset();

    void Integrate(const float deltaTime);
    void AddForce(const glm::vec3& force);

    const glm::vec4& LinearVelocity() const;
    void SetLinearVelocity(const glm::vec4& velocity);

    const glm::vec4& AngularVelocity() const;
    void SetAngularVelocity(const glm::vec4& velocity);

    TransformComponent* mTransformComponent = nullptr;
    std::vector<int> mContactIdx;
private:
    GameEntity* mEntity = nullptr; 
    float mInvMass = 0.f;
    float mInvI = 0.f;
    float mRadius = 1.f;
    glm::vec4 mLinearVelocity;
    glm::vec4 mLinearAcceleration;
    glm::vec4 mAngularVelocity;
    glm::vec3 mForceAccum;

    glm::mat4 mTransform;
    glm::mat4 mTransformInv;

    friend class PhysicSystem;
};

namespace Component{

template <>
inline const PhysicComponent UnitializedValue()
{
    return PhysicComponent();
}

template <>
inline bool Initialized(const PhysicComponent& component)
{
    return nullptr != component.mTransformComponent;
}

}

class PhysicSystem : public IComponentSystem 
{
public:
    PhysicSystem();
    virtual ~PhysicSystem();

    void Update(const float deltaTime) override;
    int CreateContact(const PhysicComponent::ContactManifold& contact);
    void RemoveContact(int idx);
    void ClearContactsCache(PhysicComponent* component);

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    PhysicsListener* m_listener = nullptr;

#if GUI_DEBUG()
    void debug_GUI() const override;
#endif

    const char* debug_name() const override { return "Physics"; }

private:
    std::vector<PhysicComponent> mComponents;
    std::vector<PhysicComponent::ContactManifold> mContactsCache;
};
