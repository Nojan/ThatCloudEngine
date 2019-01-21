#pragma once

#include "../types.hpp"
#include "../mesh_resource.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>
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
    std::vector<std::shared_ptr<MeshResourceList>> mMeshResourceList;
};