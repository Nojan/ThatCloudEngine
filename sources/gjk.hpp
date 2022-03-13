#pragma once

#include <glm/glm.hpp>
#include <vector>


class PhysShape {
public:
    virtual ~PhysShape() = default;
    virtual int FarthestPointInDirection(const glm::vec3 direction) const = 0;
    virtual const glm::vec3& GetPoint(const int idx) const = 0;
};

class PhysConvexShape : public PhysShape {
public:
    int FarthestPointInDirection(const glm::vec3 direction) const override;
    const glm::vec3& GetPoint(const int idx) const override;

    std::vector<glm::vec3> mVertices;
};

class PhysMeshShape : public PhysConvexShape {
public:
    std::vector<unsigned int> mIndex;
};

struct GjkInput {
    const PhysShape& shapeA;
    const PhysShape& shapeB;
    const glm::mat4 transformA;
    const glm::mat4 transformAinv;
    const glm::mat4 transformB;
    const glm::mat4 transformBinv;
};

GjkInput GjkMakeInput(const PhysShape& shapeA, const PhysShape& shapeB, const glm::mat4& transformA = glm::mat4(1.f), const glm::mat4& transformB = glm::mat4(1.f));

struct GjkSimplexVertex {
    glm::vec3 mVertex;
    int mIdxA = -1;
    int mIdxB = -1;
};

GjkSimplexVertex GJKSupport(const GjkInput& input, const glm::vec3& direction);

struct GjkSimplex {
    GjkSimplexVertex mVertices[4];
    glm::vec3 mLastDirection;
    float mCoeff[4] = {1.f};
    int mCount = 0;
};

void GjkSimplexContainsOrigin(GjkSimplex& simplex);

bool GJkTestIntersection(const GjkInput& input, GjkSimplex& output);

glm::vec3 GJKClosestPoint(const GjkSimplex& simplex);

void GJKClosestPointOnShape(const GjkInput& input, const GjkSimplex& simplex, glm::vec3& pointA, glm::vec3& pointB);

struct GjkContact {
    glm::vec3 position; // position on B
    glm::vec3 normal; // normal from B to A
    float distance; // negative mean penetration
};

GjkContact GJKComputeContact(const GjkInput& input);