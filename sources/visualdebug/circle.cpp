#include "circle.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

VisualDebugCircleCommand::VisualDebugCircleCommand(const glm::vec3 & position, const glm::vec3& normal, const float size, const uint subdivision, const Color::rgbap & color)
: mPosition(position)
, mNormal(normal)
, mSize(size)
, mSubdivision(subdivision)
, mColor(color)
{
    assert(0.f <= size);
    assert(3 < subdivision);
}

void VisualDebugCircleCommand::ApplyCommand(std::vector<glm::vec3>& vertexFill, std::vector<Color::rgbap>& colorFill, std::vector<uint>& indexFill, std::vector<glm::vec3>& vertexLine, std::vector<Color::rgbap>& colorLine, std::vector<uint>& indexLine) const
{
    const uint firstIndex = vertexLine.size();
    const uint subdivision = mSubdivision;
    const float subdivisionInvf = 1.f / static_cast<float>(subdivision);
    const uint vertexPerCircle = subdivision + 1;
    const float angleIncr = 2.f * glm::pi<float>() * subdivisionInvf;
    glm::mat4 rotationMat(1);
    rotationMat = glm::rotate(rotationMat, angleIncr, mNormal);
    glm::vec3 ortho(0.0f, 0.0f, 0.0f);
    {
        int min_idx = 0;
        float min_value = FLT_MAX;
        for (int idx = 0; idx < 3; ++idx)
        {
            if (mNormal[idx] < min_value)
            {
                min_value = mNormal[idx];
                min_idx = idx;
            }
        }
        ortho[min_idx] = 1.0f;
    }
    // Vertices
    for (uint i = 0; i < subdivision; ++i) {
        vertexLine.push_back(mPosition + ortho*mSize);
        colorLine.push_back(mColor);
        ortho = glm::vec3(rotationMat * glm::vec4(ortho, 1.0));
    }
    // Indexes
    const uint indexOffsetCircle = firstIndex;
    for (uint i = 1; i < subdivision; ++i) {
        indexLine.push_back(indexOffsetCircle + i - 1);
        indexLine.push_back(indexOffsetCircle + i - 0);
    }
    indexLine.push_back(indexOffsetCircle + subdivision - 1);
    indexLine.push_back(indexOffsetCircle + 0);
}
