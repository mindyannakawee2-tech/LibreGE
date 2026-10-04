#include "graphics/Texture.hpp"
#include "graphics/Window.hpp"

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

    while (!window.ShouldClose()) {
        window.PollEvents();

        window.Clear(
            25,
            25,
            30,
            255
        );

        if (image.IsValid()) {
    const float scale = 3.0f;

    const float width =
        image.GetWidth() * scale;

    const float height =
        image.GetHeight() * scale;

    const float x =
        (
            static_cast<float>(
                window.GetWidth()
            ) -
            width
        ) / 2.0f;

    const float y =
        (
            static_cast<float>(
                window.GetHeight()
            ) -
            height
        ) / 2.0f;

    image.Draw(
        x,
        y,
        width,
        height
    );
}

        window.Present();
    }

    return 0;
}
