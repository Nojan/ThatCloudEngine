#pragma once

class GameEntity;

struct PhysicEvent {
    GameEntity* a;
    GameEntity* b;
};

class PhysicsListener {
public:
    virtual void OnPhysicsEvent(PhysicEvent& e) = 0;
};
