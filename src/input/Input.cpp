#include "Input.hpp"

namespace LibreGE {

std::unordered_set<SDL_Keycode> Input::s_KeysDown;
std::unordered_set<SDL_Keycode> Input::s_KeysPressed;
std::unordered_set<SDL_Keycode> Input::s_KeysReleased;

std::unordered_set<Uint8> Input::s_MouseDown;
std::unordered_set<Uint8> Input::s_MousePressed;
std::unordered_set<Uint8> Input::s_MouseReleased;

float Input::s_MouseX = 0.0f;
float Input::s_MouseY = 0.0f;

void Input::BeginFrame() {
    s_KeysPressed.clear();
    s_KeysReleased.clear();

    s_MousePressed.clear();
    s_MouseReleased.clear();
}

void Input::ProcessEvent(
    const SDL_Event& event
) {
    switch (event.type) {

        case SDL_EVENT_KEY_DOWN:
        {
            const SDL_Keycode key =
                event.key.key;

            if (!event.key.repeat) {
                s_KeysPressed.insert(key);
            }

            s_KeysDown.insert(key);

            break;
        }

        case SDL_EVENT_KEY_UP:
        {
            const SDL_Keycode key =
                event.key.key;

            s_KeysDown.erase(key);
            s_KeysReleased.insert(key);

            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            const Uint8 button =
                event.button.button;

            s_MouseDown.insert(button);
            s_MousePressed.insert(button);

            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            const Uint8 button =
                event.button.button;

            s_MouseDown.erase(button);
            s_MouseReleased.insert(button);

            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
            s_MouseX =
                event.motion.x;

            s_MouseY =
                event.motion.y;

            break;
        }

        default:
            break;
    }
}

bool Input::IsKeyDown(
    SDL_Keycode key
) {
    return
        s_KeysDown.find(key) !=
        s_KeysDown.end();
}

bool Input::IsKeyPressed(
    SDL_Keycode key
) {
    return
        s_KeysPressed.find(key) !=
        s_KeysPressed.end();
}

bool Input::IsKeyReleased(
    SDL_Keycode key
) {
    return
        s_KeysReleased.find(key) !=
        s_KeysReleased.end();
}

bool Input::IsMouseDown(
    Uint8 button
) {
    return
        s_MouseDown.find(button) !=
        s_MouseDown.end();
}

bool Input::IsMousePressed(
    Uint8 button
) {
    return
        s_MousePressed.find(button) !=
        s_MousePressed.end();
}

bool Input::IsMouseReleased(
    Uint8 button
) {
    return
        s_MouseReleased.find(button) !=
        s_MouseReleased.end();
}

float Input::GetMouseX() {
    return s_MouseX;
}

float Input::GetMouseY() {
    return s_MouseY;
}

}
