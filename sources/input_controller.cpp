#include "input_controller.hpp"

#include "types.hpp"
#include "imgui/imgui_header.hpp"
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <SDL2/SDL.h>

constexpr bool mouseSimulateTouchEvent = false;

glm::vec2 Control2D::GetNormalizedPosition(glm::ivec2 p) const
{
    const glm::vec2 localPosition(p - position);
    const glm::vec2 normalizedPosition = glm::clamp(localPosition / glm::vec2(size), glm::vec2(0.f), glm::vec2(1.f));
    return glm::vec2(normalizedPosition - glm::vec2(0.5f)) * glm::vec2(2.0f);
}

void InputController::BeginEvents()
{
    mControl.center = false;
    mMousePositionPrevious = mMousePositionCurrent;
    if (Mode::Mouse == mMode)
    {
        mControl.view = glm::vec2(0);
        mControl.zoom = 0;
    }
    // Unset the mode for the frame
    // priority order:
    // touch
    // gamepad
    // mouse
    mMode = Mode::None;
}

void InputController::Event(const SDL_Event & e, const glm::ivec2 windowSize)
{
    if (SDL_WINDOWEVENT == e.type && SDL_WINDOWEVENT_RESIZED == e.window.event)
    {
        SetupTouchControl(windowSize);
    }
    if (SDL_KEYDOWN == e.type && SDLK_SPACE == e.key.keysym.sym)
    {
        mControl.center = true;
    }

    if (SDL_FINGERDOWN == e.type || SDL_FINGERUP == e.type || SDL_FINGERMOTION == e.type)
    {
        mMode = Mode::Touch;
        ProcessTouchEvent(e.tfinger, windowSize);
    }
    if ((Mode::None == mMode || Mode::Gamepad == mMode) && (SDL_CONTROLLERAXISMOTION == e.type || SDL_CONTROLLERBUTTONDOWN == e.type || SDL_CONTROLLERBUTTONUP == e.type))
    {
        mMode = Mode::Gamepad;
        ProcessGamepadEvent(e);
    }

    if ((Mode::None == mMode || Mode::Mouse == mMode) && (SDL_MOUSEBUTTONUP == e.type || SDL_MOUSEBUTTONDOWN == e.type || SDL_MOUSEWHEEL == e.type || SDL_MOUSEMOTION == e.type))
    {
        mMode = Mode::Mouse;
        if (mMouseClick && SDL_MOUSEBUTTONUP == e.type)
        {
            mMouseClick = false;
        }
        else if (!mMouseClick && SDL_MOUSEBUTTONDOWN == e.type)
        {
            mMouseClick = true;
        }
        if (mouseSimulateTouchEvent && (SDL_MOUSEBUTTONUP == e.type || SDL_MOUSEBUTTONDOWN == e.type || (mMouseClick && SDL_MOUSEMOTION == e.type)) )
        {
            SDL_TouchFingerEvent touch;
            touch.timestamp = e.motion.timestamp;
            touch.touchId = 1;
            touch.fingerId = 1;
            touch.x = 0;
            touch.y = 0;
            touch.dx = 0;
            touch.dy = 0;
            touch.pressure = 0.5f;
            if (SDL_MOUSEBUTTONUP == e.type)
            {
                touch.type = SDL_FINGERUP;
            }
            else if (SDL_MOUSEBUTTONDOWN == e.type)
            {
                touch.type = SDL_FINGERDOWN;
            }
            else
            {
                touch.type = SDL_FINGERMOTION;
            }
            const glm::vec2 windowSizeF(windowSize);
            const glm::vec2 position( float(e.motion.x) / windowSizeF.x,  float(e.motion.y) / windowSizeF.y);
            const glm::vec2 dposition((position.x - 0.5f) * 2.f, (position.y - 0.5f) * 2.f);
            touch.x = position.x;
            assert(fabsf(touch.x) < 2.0f);
            touch.y = position.y;
            assert(fabsf(touch.y) < 2.0f);
            touch.dx = dposition.x;
            touch.dy = dposition.y;
            ProcessTouchEvent(touch, windowSize);
        }
        if(mouseSimulateTouchEvent)
            return;
    }

    if (Mode::Mouse == mMode)
    {
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
            const float value(e.wheel.y * 15.f);
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
    }
}

void InputController::SetupTouchControl(const glm::ivec2 windowSize)
{
    // Reset
    for (size_t idx = 0; idx < mControl2D.size(); ++idx)
    {
        Control2D& c = mControl2D[idx];
        c.position = glm::ivec2(0);
        c.size = glm::ivec2(0);
        c.fingerIdx = -1;
    }

    for (size_t idx = 0; idx < mButton2D.size(); ++idx)
    {
        Button2D& b = mButton2D[idx];
        b.position = glm::ivec2(0);
        b.size = glm::ivec2(0);
        b.fingerIdx = -1;
        b.name = "";
        b.mode = Button2D::Mode::Press;
        b.state = false;
    }

    const int min_side = glm::min(windowSize.x, windowSize.y);
    const int stick_area_size(min_side * 0.45f);
    if(stick_area_size < 1)
        return;

    // Left thumb
    {
        Control2D& c = mControl2D[0];
        c.position = glm::ivec2(0, windowSize.y - stick_area_size);
        c.size = glm::ivec2(stick_area_size, stick_area_size);
    }

    // Right thumb
    {
        Control2D& c = mControl2D[1];
        c.position = glm::ivec2(windowSize.x - stick_area_size, windowSize.y - stick_area_size);
        c.size = glm::ivec2(stick_area_size, stick_area_size);
    }

    // Zoom
    {
        const float width = 0.05f;
        const int zoom_area(min_side * width);
        Control2D& c = mControl2D[2];
        c.position = glm::ivec2(windowSize.x / 2 - zoom_area, 0);
        c.size = glm::ivec2(zoom_area * 2, windowSize.y);
    }

    const int button_size(min_side * 0.1f);
    if(button_size < 1)
        return;
    // Show Control
    {
        Button2D& b = mButton2D[0];
        b.position = glm::ivec2(0, 0);
        b.size = glm::ivec2(button_size, button_size);
        b.name = "Show Control";
    }

    // Call
    {
        Button2D& b = mButton2D[1];
        b.position = glm::ivec2(windowSize.x - button_size, (button_size + 5) * 0);
        b.size = glm::ivec2(button_size, button_size);
        b.mode = Button2D::Mode::Switch;
        b.name = "Call";
    }

    // Absorb
    {
        Button2D& b = mButton2D[2];
        b.position = glm::ivec2(windowSize.x - button_size, (button_size + 5) * 1);
        b.size = glm::ivec2(button_size, button_size);
        b.name = "Absorb";
    }

    // Release
    {
        Button2D& b = mButton2D[3];
        b.position = glm::ivec2(windowSize.x - button_size, (button_size + 5) * 2);
        b.size = glm::ivec2(button_size, button_size);
        b.name = "Release";
    }
}

void InputController::ProcessTouchEvent(const SDL_TouchFingerEvent& e, const glm::ivec2 windowSize)
{
    const glm::vec2 touchPosition = glm::vec2(e.x, e.y) * glm::vec2(windowSize);
    if (SDL_FINGERDOWN == e.type)
    {
        for (size_t idx = 0; idx < mFingers.size(); ++idx)
        {
            Finger& f = mFingers[idx];
            if (Finger::State::none == f.state)
            {
                f.position = touchPosition;
                f.state = Finger::State::down;
                break;
            }
        }
    }
    else
    {
        size_t closestFinger = -1;
        float closestDistance = FLT_MAX;
        for (size_t idx = 0; idx < mFingers.size(); ++idx)
        {
            Finger& f = mFingers[idx];
            if (Finger::State::up == f.state || Finger::State::none == f.state)
            {
                continue;
            }
            const glm::vec2 diff = glm::vec2(f.position) - touchPosition;
            const float diffMagSq = glm::dot(diff, diff);
            if (diffMagSq < closestDistance)
            {
                closestDistance = diffMagSq;
                closestFinger = idx;
            }
        }
        if (-1 != closestFinger)
        {
            Finger& f = mFingers[closestFinger];
            if (SDL_FINGERUP == e.type)
            {
                f.state = Finger::State::up;
                f.position = glm::ivec2(0, 0);
            }
            else
            {
                f.state = Finger::State::motion;
                f.position = glm::ivec2(touchPosition);
            }
        }
    }
}

void InputController::ProcessTouchSurface(const glm::ivec2 & position, const glm::ivec2 & size, uint8_t& currentFingerIdx)
{
    if (uint8_t(-1) != currentFingerIdx)
    {
        // Finger leaving control
        const Finger& f = mFingers[currentFingerIdx];
        if (Finger::State::none == f.state)
        {
            currentFingerIdx = -1;
        }
        else
        {
            const glm::ivec2 diff = f.position - position;
            if (diff.x < 0 || diff.y < 0|| size.x < diff.x || size.y < diff.y)
            {
                currentFingerIdx = -1;
            }
        }
    }
    else
    {
        // Finger entering control
        for (size_t fingerIdx = 0; fingerIdx < mFingers.size(); ++fingerIdx)
        {
            const Finger& f = mFingers[fingerIdx];
            if (Finger::State::down != f.state)
                continue;
            const glm::ivec2 diff = f.position - position;
            if (diff.x >= 0 && diff.y >= 0 && size.x >= diff.x && size.y >= diff.y)
            {
                currentFingerIdx = numeric_cast<uint8_t>(fingerIdx);
                break;
            }
        }
    }
}

void InputController::ProcessGamepadEvent(const SDL_Event& e)
{
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

    for (size_t idx = 0; idx < mControl2D.size(); ++idx)
    {
        Control2D& c = mControl2D[idx];
        ProcessTouchSurface(c.position, c.size, c.fingerIdx);
    }
    for (size_t idx = 0; idx < mButton2D.size(); ++idx)
    {
        Button2D& b = mButton2D[idx];
        ProcessTouchSurface(b.position, b.size, b.fingerIdx);
        if (Button2D::Mode::Press == b.mode)
        {
            b.state = uint8_t(-1) != b.fingerIdx;
        }
        else if (Button2D::Mode::Switch == b.mode)
        {
            if (uint8_t(-1) != b.fingerIdx)
            {
                const Finger& f = mFingers[b.fingerIdx];
                if (Finger::State::down == f.state)
                {
                    b.state = !b.state;
                }
            }
        }
    }
    // All controls should all processed the up event
    for (size_t idx = 0; idx < mFingers.size(); ++idx)
    {
        Finger& f = mFingers[idx];
        if (Finger::State::up == f.state)
        {
            f.state = Finger::State::none;
        } 
        else if(Finger::State::down == f.state)
        {
            f.state = Finger::State::motion;
        }
    }
    if (mouseSimulateTouchEvent && Mode::Mouse == mMode)
    {
        mMode = Mode::Touch;
    }
    if (Mode::Touch == mMode)
    {
        auto computeControl2D = [this](const Control2D& c) -> glm::vec2
        {
            glm::vec2 result = glm::vec2(0, 0);
            if (uint8_t(-1) != c.fingerIdx)
            {
                const Finger& f = mFingers[c.fingerIdx];
                assert(Finger::State::up != f.state);
                result = c.GetNormalizedPosition(f.position);
            }
            return result;
        };
        mControl.move = computeControl2D(mControl2D[0]);
        mControl.view = computeControl2D(mControl2D[1]) * 0.01f;
        mControl.view.x = -mControl.view.x;
        mControl.zoom = computeControl2D(mControl2D[2]).y;

        mControl.call = mButton2D[1].state;
        mControl.absorb = mButton2D[2].state;
        mControl.release = mButton2D[3].state;

        if (mButton2D[0].state)
        {
            mShowTouchControl = 5.f;
        }
    }
}

void InputController::Update(const float duration)
{
    if (0 < mShowTouchControl)
    {
        mShowTouchControl -= duration;
    }
}

InputControl InputController::GetInput() const
{
    return mControl;
}

#if IMGUI_ENABLE()
void InputController::DrawGamepad()
{
    if (Mode::Touch != mMode)
        return;
    const bool showControl = 0.f < mShowTouchControl;
    const float alpha = 0.15f * glm::min(1.0f, mShowTouchControl);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    const float sz = 36.0f;
    const float thickness = 3.0f;
    const ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
    const ImU32 col = ImColor(colf);

    char name[64] = "";
    const size_t buttonsCount = showControl ? mButton2D.size() : glm::min<size_t>(1, mButton2D.size());
    for(size_t idx = 0; idx < buttonsCount; ++idx)
    {
        const Button2D& b = mButton2D[idx];
        if (0 == b.size.x)
            continue;

        const float alpha_button = b.state ? glm::min(alpha * 2.0f, 1.0f): alpha;
        ImGui::SetNextWindowPos(ImVec2(b.position.x, b.position.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(b.size.x, b.size.y), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(alpha_button);
        sprintf(name, "%s##%d", b.name.c_str(), idx);
        if (ImGui::Begin(name, nullptr, flags))
        {
            ImGui::Text("%c", b.name[0]);
        }
        ImGui::End();
    }

    if(!showControl)
        return;

    for(size_t idx = 0; idx < mControl2D.size(); ++idx)
    {
        const Control2D& c = mControl2D[idx];
        if (0 != c.size.x)
        {
            ImGui::SetNextWindowPos(ImVec2(c.position.x, c.position.y), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(c.size.x, c.size.y), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(alpha);
            sprintf(name, "stick##%d", idx);
            if (ImGui::Begin(name, nullptr, flags))
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                const ImVec2 p = ImGui::GetCursorScreenPos();
                float x = p.x + c.size.x * 0.5f, y = p.y + c.size.y * 0.5f;
                if (uint8_t(-1) != c.fingerIdx)
                {
                    draw_list->AddCircleFilled(ImVec2(x - sz*0.5f, y - sz*0.5f), sz, col, 20);
                }
                else
                {
                    draw_list->AddCircle(ImVec2(x - sz*0.5f, y - sz*0.5f), sz, col, 20, thickness);
                }
            }
            ImGui::End();
        }
    }
}
#endif

#if GUI_DEBUG()
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

