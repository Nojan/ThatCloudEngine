#pragma once

#include "../types.hpp"
#include <glm/glm.hpp>
#include <vector>

class GameEntity;

void loadlevel(const char* filepath, std::vector<GameEntity*>& entities, glm::vec3& boyPosition, glm::vec3& cameraOffset);
