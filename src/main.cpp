#include "graphics/Window.hpp"
#include "graphics/ImageLoader.hpp"

#include <SDL3/SDL.h>

#include <iostream>

int main() {
    LibreGE::Window window(
        "LibreGE",
        1280,
        720
    );

    if (!window.IsValid()) {
        return 1;
    }

    SDL_Texture* image =
        LibreGE::ImageLoader::LoadTexture(
            window.GetRenderer(),
            "assets/test.png"
        );

    if (!image) {
        std::cerr
            << "[LibreGE] Failed to load "
            << "assets/test.png\n";
    }

    while (!window.ShouldClose()) {
        window.PollEvents();

        window.Clear(
            25,
            25,
            30,
            255
        );

        if (image) {
            float imageWidth = 0.0f;
            float imageHeight = 0.0f;

            SDL_GetTextureSize(
                image,
                &imageWidth,
                &imageHeight
            );

            SDL_FRect destination {
                (
                    static_cast<float>(
                        window.GetWidth()
                    ) -
                    imageWidth
                ) / 2.0f,

                (
                    static_cast<float>(
                        window.GetHeight()
                    ) -
                    imageHeight
                ) / 2.0f,

                imageWidth,
                imageHeight
            };

            SDL_RenderTexture(
                window.GetRenderer(),
                image,
                nullptr,
                &destination
            );
        }

        window.Present();
    }

    if (image) {
        SDL_DestroyTexture(
            image
        );
    }

    return 0;
}
