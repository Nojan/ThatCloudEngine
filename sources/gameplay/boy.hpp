#pragma once

#include "../types.hpp"
#include "../iresourceowner.hpp"
#include <glm/fwd.hpp>
#include <memory>
#include <vector>

class GameEntity;
class Resource;
class ResourceMesh;

class Boy : public IResourceOwner {
public:
    Boy();
    ~Boy() = default;

    void ListResources(std::vector<Resource*>& resources) override;
    void OnLoad() override;

    void Init();
    void Terminate();

    void TeleportTo(const glm::vec3& position);
    void MoveToward(const glm::vec3& position, const float deltaTime);

    glm::vec3 Position() const;

private:
    std::unique_ptr<GameEntity> mEntity;
    std::vector<std::shared_ptr<ResourceMesh>> mResources;
};