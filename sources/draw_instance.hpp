#pragma once

#include <array>
#include <memory>

class DrawInstance {
public:
    std::shared_ptr<MeshBuffer> mMeshBuffer;
    glm::mat4 mTransform;
    glm::mat4 mTransformScale;
    std::array<std::shared_ptr<
};