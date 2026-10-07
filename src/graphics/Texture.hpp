#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace LibreGE {

class Window;
class Camera;

class Texture {
public:
    Texture(
        Window& window,
        const std::string& path
    );

    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool IsValid() const;

    void Draw(
        float x,
        float y
    );

    void Draw(
        float x,
        float y,
        float width,
        float height
    );

    void Draw(
        const Camera& camera,
        float x,
        float y
    );

    void Draw(
        const Camera& camera,
        float x,
        float y,
        float width,
        float height
    );


    void DrawCentered();

    void DrawCentered(
        float maxWidth,
        float maxHeight
    );

    float GetWidth() const;
    float GetHeight() const;

    SDL_Texture* GetNativeTexture() const;

private:
    void Release();

    Window* m_Window = nullptr;
    SDL_Texture* m_Texture = nullptr;

    float m_Width = 0.0f;
    float m_Height = 0.0f;

    std::string m_Path;
};

}
