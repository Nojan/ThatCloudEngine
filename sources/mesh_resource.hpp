#pragma once

#include <memory>
#include <vector>

class Mesh;
class Texture2D;

struct MeshResource
{
    std::shared_ptr<Mesh> m_mesh;
    std::shared_ptr<Texture2D> m_texture;
};

using MeshResourceList = std::vector<MeshResource>;
