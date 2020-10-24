#include "gjk.hpp"

#include "hedge.hpp"

float SumCoeff(const GjkSimplex& simplex)
{
    float result = 0.f;
    for (int idx = 0; idx < simplex.mCount; ++idx)
    {
        result += simplex.mCoeff[idx];
    }
    return result;
}

int PhysConvexShape::FarthestPointInDirection(const glm::vec3 direction) const
{
    int bestIdx = -1;
    float distance = -FLT_MAX;
    for (size_t idx = 0; idx < mVertices.size(); ++idx)
    {
        const float dCandidate = dot(mVertices[idx], direction);
        if (dCandidate > distance)
        {
            bestIdx = static_cast<int>(idx);
            distance = dCandidate;
        }
    }
    return bestIdx;
}

const glm::vec3& PhysConvexShape::GetPoint(const int idx) const
{
    return mVertices[idx];
}

GjkInput GjkMakeInput(const PhysShape& shapeA, const PhysShape& shapeB, const glm::mat4& transformA, const glm::mat4& transformB)
{
    assert(false == glm::any(glm::isnan(transformA[0])));
    assert(false == glm::any(glm::isnan(transformA[1])));
    assert(false == glm::any(glm::isnan(transformA[2])));
    assert(false == glm::any(glm::isnan(transformA[3])));
    assert(false == glm::any(glm::isnan(transformB[0])));
    assert(false == glm::any(glm::isnan(transformB[1])));
    assert(false == glm::any(glm::isnan(transformB[2])));
    assert(false == glm::any(glm::isnan(transformB[3])));
    GjkInput result = { shapeA, shapeB, transformA, glm::inverse(transformA), transformB, glm::inverse(transformB) };
    assert(false == glm::any(glm::isnan(result.transformAinv[0])));
    assert(false == glm::any(glm::isnan(result.transformAinv[1])));
    assert(false == glm::any(glm::isnan(result.transformAinv[2])));
    assert(false == glm::any(glm::isnan(result.transformAinv[3])));
    assert(false == glm::any(glm::isnan(result.transformBinv[0])));
    assert(false == glm::any(glm::isnan(result.transformBinv[1])));
    assert(false == glm::any(glm::isnan(result.transformBinv[2])));
    assert(false == glm::any(glm::isnan(result.transformBinv[3])));
    return result;
}

GjkSimplexVertex GJKSupport(const GjkInput& input, const glm::vec3& direction)
{
    GjkSimplexVertex vertice;
    vertice.mIdxA = input.shapeA.FarthestPointInDirection(glm::vec3(input.transformAinv * glm::vec4( direction, 0.f)));
    vertice.mIdxB = input.shapeB.FarthestPointInDirection(glm::vec3(input.transformBinv * glm::vec4(-direction, 0.f)));
    const glm::vec3& pointA = input.shapeA.GetPoint(vertice.mIdxA);
    const glm::vec3& pointB = input.shapeB.GetPoint(vertice.mIdxB);
    const glm::vec4 transformedPointA(input.transformA * glm::vec4(pointA, 1.f));
    const glm::vec4 transformedPointB(input.transformB * glm::vec4(pointB, 1.f));
    vertice.mVertex = glm::vec3(transformedPointA) - glm::vec3(transformedPointB);
    return vertice;
}

const glm::vec3 GetSearchDirection(const GjkSimplex& simplex)
{
    glm::vec3 direction = glm::vec3(0);
    switch (simplex.mCount)
    {
    case 1:
        direction = -simplex.mVertices[0].mVertex;
        break;
    case 2:
    {
        const glm::vec3& a = simplex.mVertices[0].mVertex;
        const glm::vec3& b = simplex.mVertices[1].mVertex;
        const glm::vec3 ao = -a;
        const glm::vec3 ab = b - a;
        direction = glm::cross(ab, glm::cross(ao, ab));
        break;
    }

    case 3:
    {
        const glm::vec3& a = simplex.mVertices[0].mVertex;
        const glm::vec3& b = simplex.mVertices[1].mVertex;
        const glm::vec3& c = simplex.mVertices[2].mVertex;
        const glm::vec3 ao = -a;
        const glm::vec3 ab = b - a;
        const glm::vec3 ac = c - a;
        const glm::vec3 n = glm::cross(ab, ac);
        if (0.f < glm::dot(n, ao))
        {
            direction = n;
        }
        else
        {
            direction = -n;
        }
        break;
    }
    default:
        assert(false);
    }

    return direction;
}

void GjkSimplexCheck2(GjkSimplex& simplex)
{
    assert(2 == simplex.mCount);
    const glm::vec3& a = simplex.mVertices[0].mVertex;
    const glm::vec3& b = simplex.mVertices[1].mVertex;
    const glm::vec3 ao = -a;
    const glm::vec3 ab = b - a;
    const glm::vec3 ba = a - b;
    const float u = glm::dot(b, ab);
    const float v = glm::dot(a, ba);
    if (v <= 0)
    {
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
    }
    else if (u <= 0)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
    }
    else
    {
        simplex.mCoeff[0] = u;
        simplex.mCoeff[1] = v;
    }
}

void GjkSimplexCheck3(GjkSimplex& simplex)
{
    assert(3 == simplex.mCount);
    const glm::vec3& a = simplex.mVertices[0].mVertex;
    const glm::vec3& b = simplex.mVertices[1].mVertex;
    const glm::vec3& c = simplex.mVertices[2].mVertex;

    const glm::vec3 ao = -a;
    const glm::vec3 ab = b - a;
    const glm::vec3 ba = a - b;
    const glm::vec3 ac = c - a;
    const glm::vec3 ca = a - c;
    const glm::vec3 bc = c - b;
    const glm::vec3 cb = b - c;

    const float uAB = glm::dot(b, ab);
    const float vAB = glm::dot(a, a - b);

    const float uBC = glm::dot(c, bc);
    const float vBC = glm::dot(b, cb);

    const float uCA = glm::dot(a, ca);
    const float vCA = glm::dot(c, ac);

    // region a
    if (vAB <= 0.f && uCA <= 0.f)
    {
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    // region b
    if (uAB <= 0.f && vBC <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    // region c
    if (uBC <= 0.f && vCA <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[2];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    const glm::vec3 n = glm::cross(ab, ac);
    const float wABC = glm::dot(glm::cross(a, b), n);

    // Region ab
    if (uAB > 0.f && vAB > 0.f && wABC <= 0.f)
    {
        simplex.mCoeff[0] = uAB;
        simplex.mCoeff[1] = vAB;
        simplex.mCount = 2;
        return;
    }

    const float uABC = glm::dot(glm::cross(b, c), n);

    // region bc
    if (uBC > 0.f && vBC > 0.f && uABC <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mVertices[1] = simplex.mVertices[2];
        simplex.mCoeff[0] = uBC;
        simplex.mCoeff[1] = vBC;
        simplex.mCount = 2;
        return;
    }

    const float vABC = glm::dot(glm::cross(c, a), n);

    // region ca
    if (uCA > 0.f && vCA > 0.f && vABC <= 0.f)
    {
        simplex.mVertices[1] = simplex.mVertices[0];
        simplex.mVertices[0] = simplex.mVertices[2];
        simplex.mCoeff[0] = uCA;
        simplex.mCoeff[1] = vCA;
        simplex.mCount = 2;
        return;
    }

    // region abc
    assert(uABC > 0.f && vABC > 0.f && wABC > 0.f);
    simplex.mCoeff[0] = uABC;
    simplex.mCoeff[1] = vABC;
    simplex.mCoeff[2] = wABC;
}

void GjkSimplexCheck4(GjkSimplex& simplex)
{
    assert(4 == simplex.mCount);
    const glm::vec3& a = simplex.mVertices[0].mVertex;
    const glm::vec3& b = simplex.mVertices[1].mVertex;
    const glm::vec3& c = simplex.mVertices[2].mVertex;
    const glm::vec3& d = simplex.mVertices[3].mVertex;

    const glm::vec3 ao = -a;
    const glm::vec3 ab = b - a;
    const glm::vec3 ba = a - b;
    const glm::vec3 ac = c - a;
    const glm::vec3 ca = a - c;
    const glm::vec3 bc = c - b;
    const glm::vec3 cb = b - c;
    const glm::vec3 ad = d - a;

    const float uAB = glm::dot(b, ab);
    const float vAB = glm::dot(a, ba);

    const float uBC = glm::dot(c, bc);
    const float vBC = glm::dot(b, cb);

    const float uCA = glm::dot(a, ca);
    const float vCA = glm::dot(c, ac);

    const float uBD = glm::dot(d, d - b);
    const float vBD = glm::dot(b, b - d);

    const float uDC = glm::dot(c, c - d);
    const float vDC = glm::dot(d, d - c);

    const float uAD = glm::dot(d, ad);
    const float vAD = glm::dot(a, a - d);

    // region a
    if (vAB <= 0.f && uCA <= 0.f && vAD <= 0.f)
    {
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    // region b
    if (uAB <= 0.f && vBC <= 0.f && vBD <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    // region c
    if (uBC <= 0.f && vCA <= 0.f && uDC <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[2];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    // region d
    if (uBD <= 0.f && vDC <= 0.f && uAD <= 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[3];
        simplex.mCoeff[0] = 1.f;
        simplex.mCount = 1;
        return;
    }

    const glm::vec3 nADB = glm::cross(ad, ab);
    const float uADB = glm::dot(glm::cross(d, b), nADB);
    const float vADB = glm::dot(glm::cross(b, a), nADB);
    const float wADB = glm::dot(glm::cross(a, d), nADB);

    const glm::vec3 nACD = glm::cross(ac, ad);
    const float uACD = glm::dot(glm::cross(c, d), nACD);
    const float vACD = glm::dot(glm::cross(d, a), nACD);
    const float wACD = glm::dot(glm::cross(a, c), nACD);

    const glm::vec3 nCBD = glm::cross(cb, d - c);
    const float uCBD = glm::dot(glm::cross(b, d), nCBD);
    const float vCBD = glm::dot(glm::cross(d, c), nCBD);
    const float wCBD = glm::dot(glm::cross(c, b), nCBD);

    const glm::vec3 nABC = glm::cross(ab, ac);
    const float uABC = glm::dot(glm::cross(b, c), nABC);
    const float vABC = glm::dot(glm::cross(c, a), nABC);
    const float wABC = glm::dot(glm::cross(a, b), nABC);

    // region ab
    if (wABC <= 0.f && vADB <= 0.f && uAB > 0.f && vAB > 0.f)
    {
        simplex.mCoeff[0] = uAB;
        simplex.mCoeff[1] = vAB;
        simplex.mCount = 2;
        return;
    }

    // region bc
    if (uABC <= 0.f && wCBD <= 0.f && uBC > 0.f && vBC > 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mVertices[1] = simplex.mVertices[2];
        simplex.mCoeff[0] = uBC;
        simplex.mCoeff[1] = vBC;
        simplex.mCount = 2;
        return;
    }

    // region ca
    if (vABC <= 0.f && wACD <= 0.f && uCA > 0.f && vCA > 0.f)
    {
        simplex.mVertices[1] = simplex.mVertices[0];
        simplex.mVertices[0] = simplex.mVertices[2];
        simplex.mCoeff[0] = uCA;
        simplex.mCoeff[1] = vCA;
        simplex.mCount = 2;
        return;
    }

    // region dc
    if (vCBD <= 0.f && uACD <= 0.f && uDC > 0.f && vDC > 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[3];
        simplex.mVertices[1] = simplex.mVertices[2];
        simplex.mCoeff[0] = uDC;
        simplex.mCoeff[1] = vDC;
        simplex.mCount = 2;
        return;
    }

    // region ad
    if (vACD <= 0.f && wADB <= 0.f && uAD > 0.f && vAD > 0.f)
    {
        simplex.mVertices[1] = simplex.mVertices[3];
        simplex.mCoeff[0] = uAD;
        simplex.mCoeff[1] = vAD;
        simplex.mCount = 2;
        return;
    }

    // region bd
    if (uCBD <= 0.f && uADB <= 0.f && uBD > 0.f && vBD > 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[1];
        simplex.mVertices[1] = simplex.mVertices[3];
        simplex.mCoeff[0] = uBD;
        simplex.mCoeff[1] = vBD;
        simplex.mCount = 2;
        return;
    }

    auto lmScalarTripleProduct = [](const glm::vec3& x, const glm::vec3 y, const glm::vec3 z) -> float
    {
        return glm::dot(glm::cross(x, y), z);
    };

    const float denom = lmScalarTripleProduct(bc, ba, d - b);
    const float volume = (denom == 0.f) ? 1.f : 1.f / denom;

    const float uABCD = lmScalarTripleProduct(c, d, b) * volume;
    const float vABCD = lmScalarTripleProduct(c, a, d) * volume;
    const float wABCD = lmScalarTripleProduct(d, a, b) * volume;
    const float xABCD = lmScalarTripleProduct(b, a, c) * volume;

    // region abc
    if (xABCD < 0.f && uABC > 0.f && vABC > 0.f && wABC > 0.f)
    {
        simplex.mCoeff[0] = uABC;
        simplex.mCoeff[1] = vABC;
        simplex.mCoeff[2] = wABC;
        simplex.mCount = 3;
        return;
    }

    // region cbd
    if (uABCD < 0.f && uCBD > 0.f && vCBD > 0.f && wCBD > 0.f)
    {
        simplex.mVertices[0] = simplex.mVertices[2];
        simplex.mVertices[2] = simplex.mVertices[3];
        simplex.mCoeff[0] = uCBD;
        simplex.mCoeff[1] = vCBD;
        simplex.mCoeff[2] = wCBD;
        simplex.mCount = 3;
        return;
    }

    // region acd
    if (vABCD < 0.f && uACD > 0.f && vACD > 0.f && wACD > 0.f)
    {
        simplex.mVertices[1] = simplex.mVertices[2];
        simplex.mVertices[2] = simplex.mVertices[3];
        simplex.mCoeff[0] = uACD;
        simplex.mCoeff[1] = vACD;
        simplex.mCoeff[2] = wACD;
        simplex.mCount = 3;
        return;
    }

    // region adb
    if (wABCD < 0.f && uADB > 0.f && vADB > 0.f && wADB > 0.f)
    {
        simplex.mVertices[2] = simplex.mVertices[1];
        simplex.mVertices[1] = simplex.mVertices[3];
        simplex.mCoeff[0] = uADB;
        simplex.mCoeff[1] = vADB;
        simplex.mCoeff[2] = wADB;
        simplex.mCount = 3;
        return;
    }

    // region abcd
    assert(uABCD >= 0.f && vABCD >= 0.f && wABCD >= 0.f && xABCD >= 0.f);
    simplex.mCoeff[0] = uABCD;
    simplex.mCoeff[1] = vABCD;
    simplex.mCoeff[2] = wABCD;
    simplex.mCoeff[3] = xABCD;
    //assert(1.f == uABCD + vABCD + wABCD + xABCD);
}

void GjkSimplexContainsOrigin(GjkSimplex& simplex)
{
    if (4 == simplex.mCount)
    {
        GjkSimplexCheck4(simplex);
    }
    else if (3 == simplex.mCount)
    {
        GjkSimplexCheck3(simplex);
    }
    else if (2 == simplex.mCount)
    {
        GjkSimplexCheck2(simplex);
    }
    else if (1 == simplex.mCount)
    {
        simplex.mCoeff[0] = 1.f;
    }
    assert(0.f != SumCoeff(simplex));
}

bool GJkTestIntersection(const GjkInput& input, GjkSimplex& output)
{
    glm::vec3& direction = output.mLastDirection;
    direction = glm::vec3(1.f, 0.f, 0.f);
    output.mVertices[0] = GJKSupport(input, direction);
    output.mCount = 1;
    const int endIdx = 64;
    float distance0 = FLT_MAX;
    float distance1 = FLT_MAX;
    glm::vec3 closestPoint(FLT_MAX);
    for (int i = 0; i < endIdx; ++i)
    {
        direction = GetSearchDirection(output);
        assert(0 < output.mCount);
        assert(4 > output.mCount);
        for (int idx = output.mCount; 0 < idx; --idx)
        {
            output.mVertices[idx] = output.mVertices[idx-1];
        }
        output.mVertices[0] = GJKSupport(input, direction);
        output.mCount++;

        GjkSimplexContainsOrigin(output);
        if (4 == output.mCount)
        {
            return true;
        }

        closestPoint = GJKClosestPoint(output);
        distance1 = glm::dot(closestPoint, closestPoint);
        //if (distance1 < FLT_EPSILON * FLT_EPSILON)
        //    return true;
        if(distance0 < distance1 || 0.f == distance1)
            break;
        distance0 = distance1;
    }
    return false;
}

glm::vec3 GJKClosestPoint(const GjkSimplex& simplex)
{
    glm::vec3 result = glm::vec3(0);
    float denom = 0.f;
    for (int idx = 0; idx < simplex.mCount; ++idx)
    {
        denom += simplex.mCoeff[idx];
    }
    assert(0 != denom);
    denom = 1.f / denom;
    for (int idx = 0; idx < simplex.mCount; ++idx)
    {
        const GjkSimplexVertex& simplexVertex = simplex.mVertices[idx];
        const float coeff = simplex.mCoeff[idx] * denom;
        result += simplexVertex.mVertex * coeff;
    }
    return result;
}

void GJKClosestPointOnShape(const GjkInput& input, const GjkSimplex& simplex, glm::vec3& pointA, glm::vec3& pointB)
{
    pointA = glm::vec3(0);
    pointB = glm::vec3(0);
    float denom = 0.f;
    for (int idx = 0; idx < simplex.mCount; ++idx)
    {
        denom += simplex.mCoeff[idx];
    }
    assert(0 != denom);
    denom = 1.f / denom;
    for (int idx = 0; idx < simplex.mCount; ++idx)
    {
        const GjkSimplexVertex& simplexVertex = simplex.mVertices[idx];
        const float coeff = simplex.mCoeff[idx] * denom;
        pointA += input.shapeA.GetPoint(simplexVertex.mIdxA) * coeff;
        pointB += input.shapeB.GetPoint(simplexVertex.mIdxB) * coeff;
    }

    const glm::vec4 transformedPointA(input.transformA * glm::vec4(pointA, 1.f));
    const glm::vec4 transformedPointB(input.transformB * glm::vec4(pointB, 1.f));
    pointA = glm::vec3(transformedPointA);
    pointB = glm::vec3(transformedPointB);
}

void EPAComputeFaceNormalDistance(const std::vector<GjkSimplexVertex>& vertices, const uint8_t aIdx, const uint8_t bIdx, const uint8_t cIdx, glm::vec3& normal, float& distance)
{
    const glm::vec3& a = vertices[aIdx].mVertex;
    const glm::vec3& b = vertices[bIdx].mVertex;
    const glm::vec3& c = vertices[cIdx].mVertex;

    const glm::vec3 ab = b - a;
    const glm::vec3 ac = c - a;
    normal = glm::normalize(glm::cross(ac, ab));
    distance = glm::dot(normal, a);
    if (distance < 0)
    {
        distance *= -1.f;
        normal *= -1.f;
    }
}

void EPAValidateFace(const std::vector<GjkSimplexVertex>& vertices, const uint8_t aIdx, const uint8_t bIdx, const uint8_t cIdx)
{
    glm::vec3 normal;
    float distance;
    EPAComputeFaceNormalDistance(vertices, aIdx, bIdx, cIdx, normal, distance);
    assert(0 <= distance);
}

void EPAComputeFaceNormalDistance(const std::vector<GjkSimplexVertex>& vertices, const EPAHalfEdgeMesh& mesh, const EPAHalfEdgeFace& face, glm::vec3& normal, float& distance)
{
    const EPAHalfEdge& edgeA = mesh.halfEdges[face.halfEdge];
    const EPAHalfEdge& edgeB = mesh.halfEdges[edgeA.next];
    const EPAHalfEdge& edgeC = mesh.halfEdges[edgeB.next];
    EPAComputeFaceNormalDistance(vertices, edgeA.vertex, edgeB.vertex, edgeC.vertex, normal, distance);
}

uint8_t EPAClosestFaceFromOrigin(const std::vector<GjkSimplexVertex>& vertices, const EPAHalfEdgeMesh& mesh, glm::vec3& normal, float& distance)
{
    uint8_t result = -1;
    distance = FLT_MAX;
    for (uint8_t faceIdx = 0; faceIdx < mesh.facesCount; ++faceIdx)
    {
        const EPAHalfEdgeFace& face = mesh.faces[faceIdx];
        if (uint8_t(-1) == face.halfEdge)
            continue;
        float d;
        glm::vec3 n;
        EPAComputeFaceNormalDistance(vertices, mesh, face, n, d);
        if (d < distance)
        {
            distance = d;
            normal = n;
            result = faceIdx;
        }
    }
    return result;
}

glm::vec3 barycentric_vector(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c)
{
    glm::vec3 uvw;

    const glm::vec3 v0 = b - a;
    const glm::vec3 v1 = c - a;
    const glm::vec3 v2 = p - a;

    const float d00 = glm::dot(v0, v0);
    const float d01 = glm::dot(v0, v1);
    const float d11 = glm::dot(v1, v1);
    const float d20 = glm::dot(v2, v0);
    const float d21 = glm::dot(v2, v1);
    const float invDenom = 1.f / (d00 * d11 - d01 * d01);

    uvw.y = (d11 * d20 - d01 * d21) * invDenom;
    uvw.z = (d00 * d21 - d01 * d20) * invDenom;
    uvw.x = 1.f - uvw.y - uvw.z;

    return uvw;
}

GjkContact GJKComputeContact(const GjkInput& input)
{
    GjkContact contact;
    GjkSimplex simplex;
    const bool intersection = GJkTestIntersection(input, simplex);
    if (intersection)
    {
        EPAHalfEdgeMesh mesh;
        mesh.facesCapacity = 64;
        const size_t memorySize = EPAComputedAllocatedSize(mesh);

        void* memoryBlock = malloc(memorySize);
        EPAInitialize(mesh, memoryBlock);
        std::vector<GjkSimplexVertex> vertices;
        {
            assert(4 == simplex.mCount);

            const uint8_t idxA = 0;
            const uint8_t idxB = 1;
            const uint8_t idxC = 2;
            const uint8_t idxD = 3;

            vertices.push_back(simplex.mVertices[0]);
            vertices.push_back(simplex.mVertices[1]);
            vertices.push_back(simplex.mVertices[2]);
            vertices.push_back(simplex.mVertices[3]);

            EPAValidateFace(vertices, idxA, idxB, idxC);
            EPAValidateFace(vertices, idxA, idxC, idxD);
            EPAValidateFace(vertices, idxA, idxD, idxB);
            EPAValidateFace(vertices, idxB, idxD, idxC);
            EPAPushFace(mesh, idxA, idxB, idxC);
            EPAPushFace(mesh, idxA, idxC, idxD);
            EPAPushFace(mesh, idxA, idxD, idxB);
            EPAPushFace(mesh, idxB, idxD, idxC);
        }
        std::vector<EPAEdge> contours;
        contours.reserve(6);

        constexpr float tolerance = 0.00001f;
        constexpr int max_iteration = 100;
        int i = 1;
        while (true)
        {
            glm::vec3 faceNormal;
            float faceDistance;
            const uint8_t faceIdx = EPAClosestFaceFromOrigin(vertices, mesh, faceNormal, faceDistance);
            assert(uint8_t(-1) != faceIdx);
            const GjkSimplexVertex vertex = GJKSupport(input, faceNormal);
            const float distance = glm::dot(vertex.mVertex, faceNormal);
            if ((distance - faceDistance) < tolerance || i >= max_iteration)
            {
                const EPAHalfEdgeFace& face = mesh.faces[faceIdx];
                assert(uint8_t(-1) != face.halfEdge);
                const EPAHalfEdge& edge0 = mesh.halfEdges[face.halfEdge];
                const EPAHalfEdge& edge1 = mesh.halfEdges[edge0.next];
                const EPAHalfEdge& edge2 = mesh.halfEdges[edge1.next];
                assert(edge2.next == face.halfEdge);
                const GjkSimplexVertex& v0 = vertices[edge0.vertex];
                const GjkSimplexVertex& v1 = vertices[edge1.vertex];
                const GjkSimplexVertex& v2 = vertices[edge2.vertex];
                const glm::vec3 uvw = barycentric_vector(faceNormal * faceDistance, v0.mVertex, v1.mVertex, v2.mVertex);
                const glm::vec3 v0_B(input.transformB * glm::vec4(input.shapeB.GetPoint(v0.mIdxB), 1.f));
                const glm::vec3 v1_B(input.transformB * glm::vec4(input.shapeB.GetPoint(v1.mIdxB), 1.f));
                const glm::vec3 v2_B(input.transformB * glm::vec4(input.shapeB.GetPoint(v2.mIdxB), 1.f));
                const glm::vec3 point = v0_B * uvw.x + v1_B * uvw.y + v2_B * uvw.z;
                contact.position = point;
                contact.distance = -distance;
                contact.normal = -faceNormal;
                break;
            }
            vertices.push_back(vertex);
            const uint8_t a(vertices.size() - 1);
            contours.clear();
            for (uint8_t faceIdx = 0; faceIdx < mesh.facesCount; ++faceIdx)
            {
                const EPAHalfEdgeFace& face = mesh.faces[faceIdx];
                if (uint8_t(-1) == face.halfEdge)
                    continue;
                float d;
                glm::vec3 n;
                EPAComputeFaceNormalDistance(vertices, mesh, face, n, d);
                const glm::vec3& faceVertex = vertices[mesh.halfEdges[face.halfEdge].vertex].mVertex;
                if (0.f <= glm::dot(n, vertex.mVertex - faceVertex))
                {
                    uint8_t b(-1);
                    uint8_t c(-1);
                    uint8_t d(-1);
                    // Remove the face
                    {
                        EPAHalfEdgeFace& face = mesh.faces[faceIdx];
                        assert(uint8_t(-1) != face.halfEdge);
                        EPAHalfEdge& edgeB = mesh.halfEdges[face.halfEdge];
                        EPAHalfEdge& edgeC = mesh.halfEdges[edgeB.next];
                        EPAHalfEdge& edgeD = mesh.halfEdges[edgeC.next];
                        assert(edgeD.next == face.halfEdge);
                        b = edgeB.vertex;
                        c = edgeC.vertex;
                        d = edgeD.vertex;

                        edgeB.face = -1;
                        edgeC.face = -1;
                        edgeD.face = -1;
                        face.halfEdge = -1;
                    }
                    assert(uint8_t(-1) != b);
                    assert(uint8_t(-1) != c);
                    assert(uint8_t(-1) != d);
                    auto expandContour = [](std::vector<EPAEdge>& contours, const uint8_t begin, const uint8_t end)
                    {
                        bool removed = false;
                        for (int contourIdx = int(contours.size()) - 1; 0 <= contourIdx; --contourIdx)
                        {
                            const EPAEdge& contourEdge = contours[contourIdx];
                            if ((contourEdge.vertexBegin == end && contourEdge.vertexEnd == begin))
                            {
                                contours[contourIdx] = contours[contours.size() - 1];
                                contours.resize(contours.size() - 1);
                                removed = true;
                                break;
                            }
                        }
                        if (!removed)
                        {
                            contours.push_back({ begin, end });
                        }
                    };
                    expandContour(contours, b, c);
                    expandContour(contours, c, d);
                    expandContour(contours, d, b);
                }
            }
            for (const auto& edge : contours)
            {
                EPAValidateFace(vertices, a, edge.vertexBegin, edge.vertexEnd);
                EPAPushFace(mesh, a, edge.vertexBegin, edge.vertexEnd);
            }
            ++i;
        }

        free(memoryBlock);
    }
    else
    {
        glm::vec3 pointA, pointB;
        GJKClosestPointOnShape(input, simplex, pointA, pointB);
        const glm::vec3 diff = pointA - pointB;
        contact.position = pointB;
        contact.distance = glm::length(diff);
        // when the distance is 0, the contact is the normal of shapeB at pointB
        if (0.f == contact.distance)
        {
            contact.normal = -glm::normalize(simplex.mLastDirection);
        }
        else
        {
            contact.normal = diff / contact.distance;
        }
        contact.distance = contact.distance;
    }
    return contact;
}
