#pragma once

#include "../types.hpp"
#include <glm/fwd.hpp>
#include <functional>
#include <vector>

class GameEntity;

using CloudSpawner = std::function<void (const glm::vec3&, const int, const float)>;
using GridSpawner = std::function<void (const char*, const int, const int)>;

void loadlevel(const char* filepath, CloudSpawner cloudSpawner, GridSpawner gridSpawner, glm::vec3& boyPosition, glm::vec3& cameraOffset);
