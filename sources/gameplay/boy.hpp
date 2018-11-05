#pragma once

#include "../types.hpp"
#include <glm/glm.hpp>
#include <memory>
class GameEntity;

class Boy {
public:
    Boy() = default;
    ~Boy() = default;

    void Init();
    void Terminate();

    void TeleportTo(const glm::vec3& position);
    void MoveToward(const glm::vec3& position, const float deltaTime);

    glm::vec3 Position() const;

private:
    std::unique_ptr<GameEntity> mEntity;
};