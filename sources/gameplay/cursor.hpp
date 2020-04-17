#pragma once

#include "../types.hpp"
#include "../iresourceowner.hpp"
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

class Texture2D;
class GameEntity;

class Cursor : public IResourceOwner {
public:
    Cursor();
    ~Cursor() = default;

    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    void Init();
    void Terminate();

    void SetPosition(const glm::vec3& position, const float deltaTime);

private:
    std::unique_ptr<GameEntity> mEntity;
    std::vector<std::shared_ptr<Texture2D>> mTextures;
    std::vector<std::shared_ptr<Resource>> mResources;
    size_t mIdx = 0;
    float mTimer = 0.f;
};
