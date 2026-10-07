#include "Client.hpp"

#include "../core/Time.hpp"

#include "../graphics/Camera.hpp"
#include "../graphics/Texture.hpp"
#include "../graphics/Window.hpp"

#include "../input/Input.hpp"

#include "../network/NetworkClient.hpp"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <string>

namespace LibreGE {

int Client::Run() {
    const char* environmentName =
        std::getenv(
            "LIBREGE_PLAYER_NAME"
        );

    const std::string playerName =
        environmentName
            ? environmentName
            : "Player";

    Window window(
        "LibreGE - " + playerName,
        900,
        600
    );

    if (!window.IsValid()) {
        return 1;
    }

    Texture playerTexture(
        window,
        "assets/LibreGE.png"
    );

    Camera camera;

    NetworkClient network;

    network.Connect(
        "127.0.0.1",
        25565,
        playerName
    );

    float x =
        250.0f;

    float y =
        200.0f;

    constexpr float speed =
        250.0f;

    constexpr float scale =
        2.0f;

    float sendTimer =
        0.0f;

    Time::Init();

    while (!window.ShouldClose()) {
        Time::Update();
        Input::BeginFrame();

        window.PollEvents();

        const float dt =
            Time::DeltaTime();

        if (
            Input::IsKeyDown(
                SDLK_W
            )
        ) {
            y -=
                speed * dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_S
            )
        ) {
            y +=
                speed * dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_A
            )
        ) {
            x -=
                speed * dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_D
            )
        ) {
            x +=
                speed * dt;
        }

        /*
         * Send ~30 position updates/sec.
         */
        sendTimer += dt;

        if (
            network.IsConnected() &&
            sendTimer >=
                (1.0f / 30.0f)
        ) {
            network.SendPosition(
                x,
                y
            );

            sendTimer =
                0.0f;
        }

        window.Clear(
            24,
            24,
            29,
            255
        );

        /*
         * LOCAL PLAYER = green
         */
        if (
            playerTexture.IsValid()
        ) {
            SDL_SetTextureColorMod(
                playerTexture
                    .GetNativeTexture(),
                100,
                255,
                120
            );

            playerTexture.Draw(
                camera,
                x,
                y,
                playerTexture.GetWidth()
                    * scale,
                playerTexture.GetHeight()
                    * scale
            );
        }

        /*
         * REMOTE PLAYERS = red
         */
        const auto players =
            network.GetPlayers();

        for (
            const auto& [
                id,
                player
            ] :
            players
        ) {
            if (
                id ==
                network.GetPlayerID()
            ) {
                continue;
            }

            SDL_SetTextureColorMod(
                playerTexture
                    .GetNativeTexture(),
                255,
                90,
                90
            );

            playerTexture.Draw(
                camera,
                player.x,
                player.y,
                playerTexture.GetWidth()
                    * scale,
                playerTexture.GetHeight()
                    * scale
            );
        }

        SDL_SetTextureColorMod(
            playerTexture
                .GetNativeTexture(),
            255,
            255,
            255
        );

        window.Present();
    }

    network.Disconnect();

    return 0;
}

}
