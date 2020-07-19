#include "halfcone.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

VisualDebugHalfCone::VisualDebugHalfCone(const glm::vec3& positionBottom, const glm::vec3& positionTop, const float radiusBottom, const float radiusTop, const Color::rgbap& color)
: mPositionBottom(positionBottom)
, mPositionTop(positionTop)
, mRadiusBottom(radiusBottom)
, mRadiusTop(radiusTop)
, mColor(color)
{}

void VisualDebugHalfCone::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill,
    std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const uint firstIndex = vertexFill.size();
    const uint subdivision = 12;
    const float subdivisionInvf = 1.f / static_cast<float>(subdivision);
    const uint vertexPerCircle = subdivision + 1;
    const float angleIncr = 2.f * glm::pi<float>() * subdivisionInvf;
    //top circle vertex
    vertexFill.push_back(mPositionTop);
    colorFill.push_back(mColor);

    const glm::vec3 normal = glm::normalize(mPositionTop - mPositionBottom);
    const glm::vec3 tx = glm::normalize(fabsf(normal.x) > fabsf(normal.z) ? glm::vec3(-normal.y, normal.x, 0.0) : glm::vec3(0.0, -normal.z, normal.y));
    const glm::vec3 ty = glm::cross(normal, tx);
    for (uint i = 0; i < subdivision; ++i) {
        const float angle = static_cast<float>(i) * angleIncr;
        const float cosAngle = cos(angle);
        const float sinAngle = sin(angle);
        const glm::vec3 ortho = tx * cosAngle + ty *sinAngle;
        vertexFill.push_back(mPositionTop + ortho *mRadiusTop);
        colorFill.push_back(mColor);
    }
    //bottom circle vertex
    vertexFill.push_back(mPositionBottom);
    colorFill.push_back(mColor);
    for (uint i = 0; i < subdivision; ++i) {
        const float angle = static_cast<float>(i) * angleIncr;
        const float cosAngle = cos(angle);
        const float sinAngle = sin(angle);
        const glm::vec3 ortho = tx * cosAngle + ty * sinAngle;
        vertexFill.push_back(mPositionBottom + ortho *mRadiusBottom);
        colorFill.push_back(mColor);
    }
    //top circle index
    const uint indexOffsetTopCircle = firstIndex;
    for (uint i = 1; i < subdivision; ++i) {
        indexFill.push_back(indexOffsetTopCircle + 0);
        indexFill.push_back(indexOffsetTopCircle + i + 0);
        indexFill.push_back(indexOffsetTopCircle + i + 1);
    }
    indexFill.push_back(indexOffsetTopCircle + 0);
    indexFill.push_back(indexOffsetTopCircle + subdivision);
    indexFill.push_back(indexOffsetTopCircle + 1);
    //bottom circle index
    const uint indexOffsetBottomCircle = indexOffsetTopCircle + vertexPerCircle;
    for (uint i = 1; i < subdivision; ++i) {
        indexFill.push_back(indexOffsetBottomCircle + 0);
        indexFill.push_back(indexOffsetBottomCircle + i + 1);
        indexFill.push_back(indexOffsetBottomCircle + i + 0);
    }
    indexFill.push_back(indexOffsetBottomCircle + 0);
    indexFill.push_back(indexOffsetBottomCircle + 1);
    indexFill.push_back(indexOffsetBottomCircle + subdivision);
    //trunk index 
    for (uint i = 1; i < subdivision; ++i) {
        indexFill.push_back(indexOffsetTopCircle + i);
        indexFill.push_back(indexOffsetBottomCircle + i);
        indexFill.push_back(indexOffsetTopCircle + i + 1);
        
        indexFill.push_back(indexOffsetTopCircle + i + 1);
        indexFill.push_back(indexOffsetBottomCircle + i);
        indexFill.push_back(indexOffsetBottomCircle + i + 1);
    }
    
    indexFill.push_back(indexOffsetTopCircle + subdivision);
    indexFill.push_back(indexOffsetBottomCircle + subdivision);
    indexFill.push_back(indexOffsetTopCircle + 1);
    
    indexFill.push_back(indexOffsetTopCircle + 1);
    indexFill.push_back(indexOffsetBottomCircle + subdivision);
    indexFill.push_back(indexOffsetBottomCircle + 1);
    
}
