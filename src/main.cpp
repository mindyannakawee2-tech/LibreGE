#include "graphics/Texture.hpp"
#include "graphics/Window.hpp"
#include "input/Input.hpp"

#include <SDL3/SDL.h>

int main() {
    LibreGE::Window window(
        "LibreGE",
        1280,
        720
    );

    if (!window.IsValid()) {
        return 1;
    }

    LibreGE::Texture image(
        window,
        "assets/LibreGE.png"
    );

    float x = 100.0f;
    float y = 100.0f;

    const float speed = 0.02f;
    const float scale = 3.0f;

    while (!window.ShouldClose()) {
        LibreGE::Input::BeginFrame();

        window.PollEvents();

        if (
            LibreGE::Input::IsKeyDown(
                SDLK_W
            )
        ) {
            y -= speed;
        }

        if (
            LibreGE::Input::IsKeyDown(
                SDLK_S
            )
        ) {
            y += speed;
        }

        if (
            LibreGE::Input::IsKeyDown(
                SDLK_A
            )
        ) {
            x -= speed;
        }

        if (
            LibreGE::Input::IsKeyDown(
                SDLK_D
            )
        ) {
            x += speed;
        }

        if (
            LibreGE::Input::IsKeyPressed(
                SDLK_SPACE
            )
        ) {
            SDL_Log(
                "SPACE pressed!"
            );
        }

        if (
            LibreGE::Input::IsMousePressed(
                SDL_BUTTON_LEFT
            )
        ) {
            SDL_Log(
                "Mouse click at %.1f, %.1f",
                LibreGE::Input::GetMouseX(),
                LibreGE::Input::GetMouseY()
            );
        }

        window.Clear(
            25,
            25,
            30,
            255
        );

        if (image.IsValid()) {
            image.Draw(
                x,
                y,
                image.GetWidth() * scale,
                image.GetHeight() * scale
            );
        }

        window.Present();
    }

    return 0;
}
