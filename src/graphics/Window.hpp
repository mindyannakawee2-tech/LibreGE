#pragma once

#include <SDL3/SDL.h>

#include <string>

namespace LibreGE {

class Window {
public:
    Window(
        const std::string& title,
        int width,
        int height
    );

    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    bool IsValid() const;

    void Clear(
        Uint8 r,
        Uint8 g,
        Uint8 b,
        Uint8 a = 255
    );

    void Present();

    void PollEvents();

    bool ShouldClose() const;

    void Close();

    SDL_Window* GetNativeWindow() const;

    SDL_Renderer* GetRenderer() const;

    int GetWidth() const;
    int GetHeight() const;

    const std::string& GetTitle() const;

private:
    SDL_Window* m_Window = nullptr;
    SDL_Renderer* m_Renderer = nullptr;

    std::string m_Title;

    int m_Width = 0;
    int m_Height = 0;

    bool m_ShouldClose = false;
};

}
