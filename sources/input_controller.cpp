#include "input_controller.hpp"

#include "types.hpp"
#include "imgui/imgui_header.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <SDL2/SDL.h>

void InputController::BeginEvents()
{
    mControl.center = false;
    mMousePositionPrevious = mMousePositionCurrent;
    if (Mode::Mouse == mMode)
    {
        mControl.view = glm::vec2(0);
        mControl.zoom = 0;
    } 
}

void InputController::Event(const SDL_Event & e, const glm::ivec2 windowSize)
{
    if (SDL_MOUSEBUTTONUP == e.type || SDL_MOUSEBUTTONDOWN == e.type || SDL_MOUSEWHEEL == e.type || SDL_MOUSEMOTION == e.type)
    {
        mMode = Mode::Mouse;
    }
    else if (SDL_CONTROLLERAXISMOTION == e.type)
    {
        mMode = Mode::Gamepad;
    }

    if (SDL_BUTTON_RIGHT == e.button.button)
    {
        mMousePan = SDL_MOUSEBUTTONDOWN == e.type;
    }
    if (SDL_MOUSEMOTION == e.type)
    {
        const glm::vec2 newMousePosition(static_cast<float>(e.motion.x), static_cast<float>(e.motion.y));
        mMousePositionCurrent = newMousePosition;
    }

    if (SDL_MOUSEWHEEL == e.type)
    {
        const float value(e.wheel.y * 15);
        mControl.zoom += value;
    }

    if (mControl.call)
    {
        mControl.call = !(SDL_MOUSEBUTTONUP == e.type && SDL_BUTTON_LEFT == e.button.button);
    }
    else
    {
        mControl.call = (SDL_MOUSEBUTTONDOWN == e.type && SDL_BUTTON_LEFT == e.button.button);
    }

    if (mControl.absorb)
    {
        mControl.absorb = !(SDL_KEYUP == e.type && SDLK_LSHIFT == e.key.keysym.sym);
    }
    else
    {
        mControl.absorb = (SDL_KEYDOWN == e.type && SDLK_LSHIFT == e.key.keysym.sym);
    }

    if (mControl.release)
    {
        mControl.release = !(SDL_KEYUP == e.type && SDLK_LCTRL == e.key.keysym.sym);
    }
    else
    {
        mControl.release = (SDL_KEYDOWN == e.type && SDLK_LCTRL == e.key.keysym.sym);
    }

    if (SDL_MOUSEMOTION == e.type)
    {
        const float motionx = static_cast<float>(e.motion.x - (windowSize.x / 2));
        const float motiony = static_cast<float>(e.motion.y - (windowSize.y / 2));
        const float halfWidth = static_cast<float>(windowSize.x / 2);
        const float halfHeight = static_cast<float>(windowSize.y / 2);
        mControl.move = glm::vec2(motionx / halfWidth, motiony / halfHeight);
    }

    if (SDL_KEYDOWN == e.type && SDLK_SPACE == e.key.keysym.sym)
    {
        mControl.center = true;
    }

    if (SDL_CONTROLLERAXISMOTION == e.type)
    {
        const float view_gain = 0.1f;
        const float zoom_gain = 1.0f;
        const int deadzone = 8000;
        const float max_range = numeric_cast<float>(32767 - deadzone);
        const bool value_positive = 0 <= e.caxis.value;
        int value_abs = abs(e.caxis.value);
        value_abs = deadzone < value_abs ? value_abs - deadzone : 0;
        float value = numeric_cast<float>(value_abs) / max_range;
        if (!value_positive) value *= -1.0f;
        if (SDL_CONTROLLER_AXIS_LEFTX == e.caxis.axis)
        {
            mControl.move.x = value;
        } 
        else if (SDL_CONTROLLER_AXIS_LEFTY == e.caxis.axis)
        {
            mControl.move.y = value;
        }
        else if (SDL_CONTROLLER_AXIS_RIGHTX == e.caxis.axis)
        {
            mControl.view.x = value * view_gain;
        } 
        else if (SDL_CONTROLLER_AXIS_RIGHTY == e.caxis.axis)
        {
            mControl.view.y = value * view_gain;
        }
        else if (SDL_CONTROLLER_AXIS_TRIGGERRIGHT == e.caxis.axis)
        {
            mControl.zoom = value;
        } 
        else if (SDL_CONTROLLER_AXIS_TRIGGERLEFT == e.caxis.axis)
        {
            mControl.zoom = -value;
        }
    }

    if (SDL_CONTROLLERBUTTONDOWN == e.type || SDL_CONTROLLERBUTTONUP == e.type)
    {
        const bool pressed = SDL_CONTROLLERBUTTONDOWN == e.type;
        if (SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B == e.cbutton.button)
        {
            mControl.call = pressed;
        }
        if (SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A == e.cbutton.button)
        {
            mControl.release = pressed;
        }
        if (SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y == e.cbutton.button)
        {
            mControl.absorb = pressed;
        }
        if (SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X == e.cbutton.button)
        {
            mControl.center = pressed;
        }
    }
}

void InputController::EndEvents()
{
    if (mMousePan)
    {
        const float gain = 0.005f;
        mControl.view = (mMousePositionCurrent - mMousePositionPrevious)*gain;
    }
}

InputControl InputController::GetInput() const
{
    return mControl;
}

#ifdef IMGUI_ENABLE
void InputController::debug_GUI()
{
    ImGui::Text("Move %s", glm::to_string(mControl.move).c_str());
    ImGui::Text("View %s", glm::to_string(mControl.view).c_str());
    ImGui::Value("Zoom", mControl.zoom);
    ImGui::Value("Call", mControl.call);
    ImGui::Value("Absorb", mControl.absorb);
    ImGui::Value("Release", mControl.release);
}
#endif