#pragma once

#include "../types.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Texture2D;
class GameEntity;

class Cursor {
public:
    Cursor() = default;
    ~Cursor() = default;

    void Init();
    void Terminate();

    void SetPosition(const glm::vec3& position, const float deltaTime);

private:
    std::unique_ptr<GameEntity> mEntity;
    std::vector<std::shared_ptr<Texture2D>> mTextures;
    size_t mIdx = 0;
    float mTimer = 0.f;
};
