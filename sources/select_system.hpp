#pragma once

#include "icomponentsystem.hpp"

#include <glm/fwd.hpp>
#include <vector>


class SelectComponent
{
public:
    SelectComponent();
    SelectComponent(const SelectComponent& ref);

    void Initialize();
    void Invalidate();

    bool Invalid() const;

    GameEntity* mEntity = nullptr;
};

namespace Component {

    template <>
    inline const SelectComponent UnitializedValue()
    {
        return SelectComponent();
    }

    template <>
    inline bool Initialized(const SelectComponent& component)
    {
        return !component.Invalid();
    }

}

class SelectSystem : public IComponentSystem {
public:
    SelectSystem();
    virtual ~SelectSystem();

    void SelectWithRay(const glm::vec3& start, const glm::vec3& end);
    void Unselect();
    GameEntity* GetSelected();

    void FrameStep() override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

    const char* debug_name() const override { return "Select"; }

private:
    std::vector<SelectComponent> mComponents;
    size_t mSelected = -1;
};
