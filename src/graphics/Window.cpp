#include "Window.hpp"
#include "../input/Input.hpp"

#include <iostream>

namespace LibreGE {

Window::Window(
    const std::string& title,
    int width,
    int height
)
    : m_Title(title),
      m_Width(width),
      m_Height(height) {

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr
            << "[LibreGE::Window] SDL_Init failed: "
            << SDL_GetError()
            << '\n';

        return;
    }

    if (!SDL_CreateWindowAndRenderer(
            m_Title.c_str(),
            m_Width,
            m_Height,
            0,
            &m_Window,
            &m_Renderer
        )) {
        std::cerr
            << "[LibreGE::Window] "
            << "SDL_CreateWindowAndRenderer failed: "
            << SDL_GetError()
            << '\n';

        m_Window = nullptr;
        m_Renderer = nullptr;

        return;
    }

    std::cout
        << "[LibreGE::Window] Created "
        << m_Width
        << "x"
        << m_Height
        << " window: "
        << m_Title
        << '\n';
}

Window::~Window() {
    if (m_Renderer) {
        SDL_DestroyRenderer(
            m_Renderer
        );

        m_Renderer = nullptr;
    }

    if (m_Window) {
        SDL_DestroyWindow(
            m_Window
        );

        m_Window = nullptr;
    }

    SDL_Quit();
}

bool Window::IsValid() const {
    return
        m_Window != nullptr &&
        m_Renderer != nullptr;
}

void Window::Clear(
    Uint8 r,
    Uint8 g,
    Uint8 b,
    Uint8 a
) {
    if (!m_Renderer) {
        return;
    }

    SDL_SetRenderDrawColor(
        m_Renderer,
        r,
        g,
        b,
        a
    );

    SDL_RenderClear(
        m_Renderer
    );
}

void Window::Present() {
    if (!m_Renderer) {
        return;
    }

    SDL_RenderPresent(
        m_Renderer
    );
}

void Window::PollEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        Input::ProcessEvent(event);

        if (
            event.type ==
            SDL_EVENT_QUIT
        ) {
            m_ShouldClose = true;
        }

        if (
            event.type ==
                SDL_EVENT_KEY_DOWN &&
            event.key.key ==
                SDLK_ESCAPE
        ) {
            m_ShouldClose = true;
        }

        if (
            event.type ==
            SDL_EVENT_WINDOW_RESIZED
        ) {
            m_Width =
                event.window.data1;

            m_Height =
                event.window.data2;
        }
    }
}

bool Window::ShouldClose() const {
    return m_ShouldClose;
}

void Window::Close() {
    m_ShouldClose = true;
}

SDL_Window* Window::GetNativeWindow() const {
    return m_Window;
}

SDL_Renderer* Window::GetRenderer() const {
    return m_Renderer;
}

int Window::GetWidth() const {
    return m_Width;
}

int Window::GetHeight() const {
    return m_Height;
}

const std::string& Window::GetTitle() const {
    return m_Title;
}

}
