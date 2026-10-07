#pragma once

#include <SDL3/SDL.h>

#include <unordered_set>

namespace LibreGE {

class Input {
public:
    static void BeginFrame();
    static void ProcessEvent(const SDL_Event& event);

    static bool IsKeyDown(SDL_Keycode key);
    static bool IsKeyPressed(SDL_Keycode key);
    static bool IsKeyReleased(SDL_Keycode key);

    static bool IsMouseDown(Uint8 button);
    static bool IsMousePressed(Uint8 button);
    static bool IsMouseReleased(Uint8 button);

    static float GetMouseX();
    static float GetMouseY();

private:
    static std::unordered_set<SDL_Keycode> s_KeysDown;
    static std::unordered_set<SDL_Keycode> s_KeysPressed;
    static std::unordered_set<SDL_Keycode> s_KeysReleased;

    static std::unordered_set<Uint8> s_MouseDown;
    static std::unordered_set<Uint8> s_MousePressed;
    static std::unordered_set<Uint8> s_MouseReleased;

    static float s_MouseX;
    static float s_MouseY;
};

}
