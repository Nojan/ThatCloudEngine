#include "collider_resource.hpp"

#include "gjk.hpp"
#include "renderableMesh.hpp"

std::unique_ptr<PhysShape> MakeCollider(const ColliderDescriptor& descriptor)
{
    std::unique_ptr<PhysConvexShape> result;
    if (descriptor.asConvex)
    {
        auto findVertice = [](const std::vector<glm::vec3>& collection, const glm::vec3& vertex) -> size_t
        {
            size_t result = -1;
            for (size_t idx = 0; idx < collection.size(); ++idx)
            {
                const glm::vec3 diff = collection[idx] - vertex;
                const float magnitude = glm::dot(diff, diff);
                if (magnitude < 0.0001f)
                {
                    result = idx;
                    break;
                }
            }
            return result;
        };

        std::unique_ptr<PhysConvexShape> shape = std::make_unique<PhysConvexShape>();
        for (const Mesh* mesh : descriptor.mesh)
        {
            for (const glm::vec3& vertex : mesh->mVertex)
            {
                if (size_t(-1) == findVertice(shape->mVertices, vertex))
                {
                    shape->mVertices.push_back(vertex);
                }
            }
        }
        result = std::move(shape);
    }
    else
    {
        std::unique_ptr<PhysMeshShape> shape = std::make_unique<PhysMeshShape>();
        for (const Mesh* mesh : descriptor.mesh)
        {
            const uint startIndex = shape->mIndex.size();
            for (const glm::vec3& vertex : mesh->mVertex)
            {
                shape->mVertices.push_back(vertex);
            }
            for (const uint index : mesh->mIndex)
            {
                shape->mIndex.push_back(startIndex + index);
            }
        }
        result = std::move(shape);
    }
    return result;
}

