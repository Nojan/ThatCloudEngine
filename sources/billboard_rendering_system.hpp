#pragma once

#include "billboard.hpp"
#include "icomponentsystem.hpp"
#include "igraphic_component.hpp"
#include "types.hpp"

#include <memory>
#include <vector>

class BillboardRenderer;

class BillboardComponent : public IGraphicComponent<BillboardRenderer>
{
public:
    ~BillboardComponent() = default;
    void draw(BillboardRenderer* renderer, const glm::vec3& normal);

    Billboard mBillboard;
};

class BillboardRenderingSystem : public IComponentSystem {
public:
    BillboardRenderingSystem() = default;
    virtual ~BillboardRenderingSystem() = default;

    void FrameStep() override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

private:
    std::vector<std::unique_ptr<BillboardComponent>> mComponents;
    BillboardRenderer* mRenderer = nullptr;
};
