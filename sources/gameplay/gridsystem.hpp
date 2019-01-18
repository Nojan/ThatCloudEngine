#pragma once

#include "../icomponentsystem.hpp"
#include "../boundingbox.hpp"

#include <vector>

class TransformComponent;

class GridCellComponent 
{
public:
    GridCellComponent() = default;
    ~GridCellComponent() = default;

    void Update(const float deltaTime);
    BoundingBox3D GetBoundingBox() const;

    TransformComponent* mTransform = nullptr;
    bool mIsFilled = false;
};

namespace Component{

    template <>
    inline const GridCellComponent UnitializedValue()
    {
        return GridCellComponent();
    }

    template <>
    inline bool Initialized(const GridCellComponent& component)
    {
        return nullptr != component.mTransform;
    }

}

class GridCellSystem : public IComponentSystem {
public:
    GridCellSystem();
    ~GridCellSystem() = default;

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "GridCell"; }

private:
    std::vector<GridCellComponent> mComponents;
};
