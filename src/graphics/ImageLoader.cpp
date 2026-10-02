#include "ImageLoader.hpp"

#include <SDL3/SDL.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace LibreGE {

ImageData ImageLoader::Load(
    const std::string& path,
    bool flipVertical
) {
    ImageData image;

    stbi_set_flip_vertically_on_load(
        flipVertical ? 1 : 0
    );

    int originalChannels = 0;

    unsigned char* data = stbi_load(
        path.c_str(),
        &image.width,
        &image.height,
        &originalChannels,
        STBI_rgb_alpha
    );

    if (!data) {
        std::cerr
            << "[LibreGE::ImageLoader] Failed to load image: "
            << path
            << '\n';

        if (stbi_failure_reason()) {
            std::cerr
                << "[LibreGE::ImageLoader] "
                << stbi_failure_reason()
                << '\n';
        }

        return {};
    }

    // Image data returned to LibreGE is always RGBA.
    image.channels = 4;

    const std::size_t size =
        static_cast<std::size_t>(image.width) *
        static_cast<std::size_t>(image.height) *
        4;

    image.pixels.assign(
        data,
        data + size
    );

    stbi_image_free(data);

    SDL_Log(
        "Image loaded: %s (%dx%d)",
        path.c_str(),
        image.width,
        image.height
    );

    return image;
}

SDL_Texture* ImageLoader::CreateTexture(
    SDL_Renderer* renderer,
    const ImageData& image
) {
    if (!renderer) {
        SDL_Log(
            "ImageLoader::CreateTexture: renderer is null"
        );

        return nullptr;
    }

    if (!image.valid()) {
        SDL_Log(
            "ImageLoader::CreateTexture: invalid image"
        );

        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        image.width,
        image.height
    );

    if (!texture) {
        SDL_Log(
            "SDL_CreateTexture failed: %s",
            SDL_GetError()
        );

        return nullptr;
    }

    const int pitch =
        image.width * 4;

    if (!SDL_UpdateTexture(
            texture,
            nullptr,
            image.pixels.data(),
            pitch
        )) {
        SDL_Log(
            "SDL_UpdateTexture failed: %s",
            SDL_GetError()
        );

        SDL_DestroyTexture(texture);

        return nullptr;
    }

    SDL_SetTextureBlendMode(
        texture,
        SDL_BLENDMODE_BLEND
    );

    return texture;
}

SDL_Texture* ImageLoader::LoadTexture(
    SDL_Renderer* renderer,
    const std::string& path,
    bool flipVertical
) {
    ImageData image =
        Load(path, flipVertical);

    if (!image.valid()) {
        return nullptr;
    }

    return CreateTexture(
        renderer,
        image
    );
}

}
