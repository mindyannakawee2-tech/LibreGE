#pragma once

#include <SDL3/SDL.h>

#include <cstdint>
#include <string>
#include <vector>

namespace LibreGE {

struct ImageData {
    int width = 0;
    int height = 0;
    int channels = 0;

    std::vector<std::uint8_t> pixels;

    bool valid() const {
        return width > 0 &&
               height > 0 &&
               !pixels.empty();
    }
};

class ImageLoader {
public:
    // Load image into CPU memory as RGBA8.
    static ImageData Load(
        const std::string& path,
        bool flipVertical = false
    );

    // Load image and immediately create an SDL texture.
    static SDL_Texture* LoadTexture(
        SDL_Renderer* renderer,
        const std::string& path,
        bool flipVertical = false
    );

    // Create SDL texture from existing ImageData.
    static SDL_Texture* CreateTexture(
        SDL_Renderer* renderer,
        const ImageData& image
    );
};

}
