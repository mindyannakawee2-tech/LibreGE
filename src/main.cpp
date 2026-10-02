#include <SDL3/SDL.h>

#include "graphics/ImageLoader.hpp"

#include <iostream>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr
            << "SDL_Init failed: "
            << SDL_GetError()
            << '\n';

        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
            "LibreGE",
            1280,
            720,
            0,
            &window,
            &renderer
        )) {
        std::cerr
            << "SDL_CreateWindowAndRenderer failed: "
            << SDL_GetError()
            << '\n';

        SDL_Quit();
        return 1;
    }

    // ========================================================
    // FIXED IMAGE
    // ========================================================

    const char* imagePath =
        "assets/LibreGE.png";

    SDL_Texture* imageTexture =
        LibreGE::ImageLoader::LoadTexture(
            renderer,
            imagePath
        );

    if (!imageTexture) {
        std::cerr
            << "Could not load fixed image: "
            << imagePath
            << '\n';
    }

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            if (
                event.type == SDL_EVENT_KEY_DOWN &&
                event.key.key == SDLK_ESCAPE
            ) {
                running = false;
            }
        }

        SDL_SetRenderDrawColor(
            renderer,
            25,
            25,
            30,
            255
        );

        SDL_RenderClear(renderer);

        if (imageTexture) {
            float textureWidth = 0.0f;
            float textureHeight = 0.0f;

            SDL_GetTextureSize(
                imageTexture,
                &textureWidth,
                &textureHeight
            );

            const float drawWidth =
                textureWidth;

            const float drawHeight =
                textureHeight;

            SDL_FRect destination {
                (1280.0f - drawWidth * 3) / 2.0f,
                (720.0f - drawHeight * 3) / 2.0f,
                drawWidth * 3,
                drawHeight * 3
            };

            SDL_RenderTexture(
                renderer,
                imageTexture,
                nullptr,
                &destination
            );
        }

        SDL_RenderPresent(renderer);
    }

    if (imageTexture) {
        SDL_DestroyTexture(
            imageTexture
        );
    }

    SDL_DestroyRenderer(
        renderer
    );

    SDL_DestroyWindow(
        window
    );

    SDL_Quit();

    return 0;
}
