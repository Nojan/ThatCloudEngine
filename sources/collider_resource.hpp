#pragma once
#include <memory>
#include <span>

class Mesh;
class PhysShape;

struct ColliderDescriptor {
    std::span<const Mesh*> mesh;
    bool asConvex = true;
};

std::unique_ptr<PhysShape> MakeCollider(const ColliderDescriptor& descriptor);
