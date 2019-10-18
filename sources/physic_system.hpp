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
    PhysicComponent();
    PhysicComponent(const PhysicComponent& ref);

    bool IsValid() const;
    bool IsAsleep() const;

    bool HasFiniteMass() const;
    void SetMass(const float mass);
    void SetRadius(const float radius);

    void Reset();

    void Integrate(const float deltaTime);
    void AddForce(const glm::vec3& force);

    const glm::vec4& LinearVelocity() const;
    void SetLinearVelocity(const glm::vec4& velocity);

    const glm::vec4& AngularVelocity() const;
    void SetAngularVelocity(const glm::vec4& velocity);

    TransformComponent* mTransformComponent;
private:
    void SetAwake();
    GameEntity* mEntity; 
    float mInvMass;
    float mRadius = 0.0f;
    float mAwake = 0.0f;
    glm::vec4 mLinearVelocity;
    glm::vec4 mLinearAcceleration;
    glm::vec4 mAngularVelocity;
    glm::vec3 mForceAccum;

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

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    PhysicsListener* m_listener = nullptr;

#ifdef IMGUI_ENABLE
    virtual void debug_GUI() const;
#endif
    const char* debug_name() const override { return "Physics"; }

private:
    std::vector<PhysicComponent> mComponents;
};
