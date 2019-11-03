#pragma once

#include <glm/glm.hpp>

struct InputControl {
    glm::vec2 move = glm::vec2(0,0);
    glm::vec2 view = glm::vec2(0,0);
    float zoom = 0;
    bool call = false;
    bool absorb = false;
    bool release = false;
    bool center = false;
};
