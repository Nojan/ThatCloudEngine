#pragma once

#include "icomponentsystem.hpp"

#include <memory>
#include <vector>

class Texture2D;
class GraphicMeshComponent;

class AnimatedTextureComponent 
{
public:
    AnimatedTextureComponent();
    ~AnimatedTextureComponent() = default;

    void Update(const float deltaTime);

    std::vector<std::shared_ptr<Texture2D>> mTexture;
    size_t mIdx;
    GraphicMeshComponent* mGraphicComponent;
    float mTimer;
};

class AnimatedTextureSystem : public IComponentSystem {
public:
    AnimatedTextureSystem() = default;
    ~AnimatedTextureSystem() = default;

    void Update(const float deltaTime) override;

    void attachEntity(GameEntity* entity) override;
    void detachEntity(GameEntity* entity) override;

#ifdef IMGUI_ENABLE
    void debug_GUI() const override;
#endif
    const char* debug_name() const override { return "AnimatedTexture"; }

private:
    std::vector<std::unique_ptr<AnimatedTextureComponent>> mComponents;
};
