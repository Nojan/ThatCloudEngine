#pragma once
#include <glm/fwd.hpp>

class GameEntity;

struct PhysicEvent {
    GameEntity* a = nullptr;
    GameEntity* b = nullptr;
    glm::vec4* ciVelocity = nullptr;
};

class PhysicsListener {
public:
    virtual void OnPhysicsEvent(PhysicEvent& e) = 0;
};
