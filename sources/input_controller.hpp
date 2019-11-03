#pragma once

#include "config.hpp"
#include "input_control.hpp"

union SDL_Event;

class InputController {
public:
    enum class Mode {
        Mouse,
        Gamepad,
    };

    void BeginEvents();
    void Event(const SDL_Event& e, const glm::ivec2 windowSize);
    void EndEvents();
    InputControl GetInput() const;

#if GUI_DEBUG()
    void debug_GUI();
#endif

private:
    InputControl mControl;
    glm::vec2 mMousePositionPrevious = glm::vec2(0,0);
    glm::vec2 mMousePositionCurrent = glm::vec2(0,0);
    Mode mMode = Mode::Mouse;
    bool mMousePan = false;
};
