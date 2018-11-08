#pragma once

#include "../icomponentsystem.hpp"

#include <vector>

class CloudComponent 
{
public:
    CloudComponent() = default;
    ~CloudComponent() = default;

    void Update(const float deltaTime);

    int mColor = 0;
    float mPower = -1.f;
};

namespace Component{

    template <>
    inline const CloudComponent UnitializedValue()
    {
        return CloudComponent();
    }

    template <>
    inline bool Initialized(const CloudComponent& component)
    {
        return component.mPower < 0.f;
    }

}

class CloudSystem : public IComponentSystem {
public:
    CloudSystem() = default;
    ~CloudSystem() = default;

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "Cloud"; }

private:
    std::vector<CloudComponent> mComponents;
};
