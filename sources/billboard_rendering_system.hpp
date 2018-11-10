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
    void draw(BillboardRenderer* renderer);

    std::vector<Billboard> mBillboards;
};

class BillboardRenderingSystem : public IComponentSystem {
public:
    BillboardRenderingSystem() = default;
    virtual ~BillboardRenderingSystem() = default;

    void FrameStep() override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "BillboardRendering"; }

private:
    std::vector<std::unique_ptr<BillboardComponent>> mComponents;
    BillboardRenderer* mRenderer = nullptr;
};
