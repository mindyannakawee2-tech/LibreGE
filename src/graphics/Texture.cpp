#include "Texture.hpp"
#include "Camera.hpp"

#include "ImageLoader.hpp"
#include "Window.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

namespace LibreGE {

Texture::Texture(
    Window& window,
    const std::string& path
)
    : m_Window(&window),
      m_Path(path) {

    m_Texture =
        ImageLoader::LoadTexture(
            window.GetRenderer(),
            path
        );

    if (!m_Texture) {
        std::cerr
            << "[LibreGE::Texture] Failed to load texture: "
            << path
            << '\n';

        return;
    }

    if (!SDL_GetTextureSize(
            m_Texture,
            &m_Width,
            &m_Height
        )) {

        std::cerr
            << "[LibreGE::Texture] Failed to get texture size: "
            << SDL_GetError()
            << '\n';

        Release();

        return;
    }

    std::cout
        << "[LibreGE::Texture] Loaded "
        << path
        << " ("
        << m_Width
        << "x"
        << m_Height
        << ")\n";
}

Texture::~Texture() {
    Release();
}

Texture::Texture(
    Texture&& other
) noexcept
    : m_Window(other.m_Window),
      m_Texture(other.m_Texture),
      m_Width(other.m_Width),
      m_Height(other.m_Height),
      m_Path(std::move(other.m_Path)) {

    other.m_Window = nullptr;
    other.m_Texture = nullptr;
    other.m_Width = 0.0f;
    other.m_Height = 0.0f;
}

Texture& Texture::operator=(
    Texture&& other
) noexcept {
    if (this == &other) {
        return *this;
    }

    Release();

    m_Window = other.m_Window;
    m_Texture = other.m_Texture;
    m_Width = other.m_Width;
    m_Height = other.m_Height;
    m_Path = std::move(other.m_Path);

    other.m_Window = nullptr;
    other.m_Texture = nullptr;
    other.m_Width = 0.0f;
    other.m_Height = 0.0f;

    return *this;
}

void Texture::Release() {
    if (m_Texture) {
        SDL_DestroyTexture(
            m_Texture
        );

        m_Texture = nullptr;
    }
}

bool Texture::IsValid() const {
    return
        m_Window != nullptr &&
        m_Texture != nullptr;
}

void Texture::Draw(
    float x,
    float y
) {
    Draw(
        x,
        y,
        m_Width,
        m_Height
    );
}

void Texture::Draw(
    float x,
    float y,
    float width,
    float height
) {
    if (!IsValid()) {
        return;
    }

    SDL_FRect destination {
        x,
        y,
        width,
        height
    };

    SDL_RenderTexture(
        m_Window->GetRenderer(),
        m_Texture,
        nullptr,
        &destination
    );
}


void Texture::Draw(
    const Camera& camera,
    float x,
    float y
) {
    Draw(
        camera,
        x,
        y,
        m_Width,
        m_Height
    );
}

void Texture::Draw(
    const Camera& camera,
    float x,
    float y,
    float width,
    float height
) {
    if (!IsValid()) {
        return;
    }

    const float screenX =
        camera.WorldToScreenX(
            x
        );

    const float screenY =
        camera.WorldToScreenY(
            y
        );

    const float screenWidth =
        camera.Scale(
            width
        );

    const float screenHeight =
        camera.Scale(
            height
        );

    Draw(
        screenX,
        screenY,
        screenWidth,
        screenHeight
    );
}

void Texture::DrawCentered() {
    if (!IsValid()) {
        return;
    }

    const float x =
        (
            static_cast<float>(
                m_Window->GetWidth()
            ) -
            m_Width
        ) / 2.0f;

    const float y =
        (
            static_cast<float>(
                m_Window->GetHeight()
            ) -
            m_Height
        ) / 2.0f;

    Draw(
        x,
        y
    );
}

void Texture::DrawCentered(
    float maxWidth,
    float maxHeight
) {
    if (!IsValid()) {
        return;
    }

    float scale = 1.0f;

    if (m_Width > maxWidth) {
        scale =
            maxWidth /
            m_Width;
    }

    if (
        m_Height * scale >
        maxHeight
    ) {
        scale =
            maxHeight /
            m_Height;
    }

    const float drawWidth =
        m_Width * scale;

    const float drawHeight =
        m_Height * scale;

    const float x =
        (
            static_cast<float>(
                m_Window->GetWidth()
            ) -
            drawWidth
        ) / 2.0f;

    const float y =
        (
            static_cast<float>(
                m_Window->GetHeight()
            ) -
            drawHeight
        ) / 2.0f;

    Draw(
        x,
        y,
        drawWidth,
        drawHeight
    );
}

float Texture::GetWidth() const {
    return m_Width;
}

float Texture::GetHeight() const {
    return m_Height;
}

SDL_Texture* Texture::GetNativeTexture() const {
    return m_Texture;
}

}
