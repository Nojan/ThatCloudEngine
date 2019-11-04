#pragma once

#include "config.hpp"
#include "input_control.hpp"

#include <array>
#include <vector>

union SDL_Event;
struct SDL_TouchFingerEvent;

struct Finger {
    enum class State : uint8_t {
        down,
        up,
        motion,
    };
    glm::ivec2 position = glm::ivec2(0, 0);
    State state = State::up;
};

struct Control2D {
    glm::ivec2 position = glm::ivec2(0, 0);
    glm::ivec2 size = glm::ivec2(0, 0);
    uint8_t fingerIdx = -1;

    glm::vec2 GetNormalizedPosition(glm::ivec2 position) const;
};

class InputController {
public:
    enum class Mode {
        None,
        Mouse,
        Gamepad,
        Touch,
    };

    void BeginEvents();
    void Event(const SDL_Event& e, const glm::ivec2 windowSize);
    void EndEvents();
    InputControl GetInput() const;
    void SetupTouchControl(const glm::ivec2 windowSize);

#if IMGUI_ENABLE()
    void DrawGamepad();
#endif

#if GUI_DEBUG()
    void debug_GUI();
#endif

private:
    void ProcessTouchEvent(const SDL_TouchFingerEvent& e, const glm::ivec2 windowSize);
    void ProcessGamepadEvent(const SDL_Event& e);
    InputControl mControl;
    glm::vec2 mMousePositionPrevious = glm::vec2(0,0);
    glm::vec2 mMousePositionCurrent = glm::vec2(0,0);
    Mode mMode = Mode::Mouse;
    bool mMouseClick = false;
    bool mMousePan = false;

    std::array<Finger, 4> mFingers;
    std::array<Control2D, 3> mControl2D;
};
