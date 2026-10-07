#include "Client.hpp"

#include "../core/LayerManager.hpp"
#include "../core/Time.hpp"

#include "../graphics/Camera.hpp"
#include "../graphics/Texture.hpp"
#include "../graphics/Window.hpp"

#include "../input/Input.hpp"

#include "../network/NetworkClient.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace LibreGE {

// ============================================================
// Testing color
// ============================================================

struct PlayerColor {
    Uint8 r;
    Uint8 g;
    Uint8 b;
};

/*
 * Deterministic colors.
 *
 * The same player ID gets the same color
 * on every connected client.
 */
static PlayerColor GetPlayerColor(
    int playerID
) {
    static constexpr PlayerColor colors[] = {
        { 100, 255, 120 }, // green
        { 255, 100, 100 }, // red
        { 100, 160, 255 }, // blue
        { 255, 220, 100 }, // yellow
        { 210, 100, 255 }, // purple
        { 100, 255, 240 }, // cyan
        { 255, 150, 70  }, // orange
        { 255, 120, 210 }, // pink
        { 170, 255, 80  }, // lime
        { 140, 120, 255 }  // violet
    };

    constexpr int colorCount =
        sizeof(colors) /
        sizeof(colors[0]);

    /*
     * Player IDs begin at 1.
     *
     * Keep modulo safe even if ID
     * is temporarily invalid.
     */
    if (playerID <= 0) {
        return {
            220,
            220,
            220
        };
    }

    return colors[
        (playerID - 1) %
        colorCount
    ];
}

// ============================================================
// Player render entry
// ============================================================

struct PlayerRenderEntry {
    int id = -1;

    std::string name;

    float x = 0.0f;
    float y = 0.0f;

    bool local = false;

    PlayerColor color {
        255,
        255,
        255
    };
};

// ============================================================
// Client
// ============================================================

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

    // ========================================================
    // Engine Layers
    // ========================================================

    LayerManager layers;

    layers.AddLayer(
        "Background",
        0
    );

    layers.AddLayer(
        "World",
        100
    );

    layers.AddLayer(
        "Entities",
        200
    );

    layers.AddLayer(
        "UI",
        1000
    );

    // ========================================================
    // Local player
    // ========================================================

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

    // ========================================================
    // Game Loop
    // ========================================================

    while (!window.ShouldClose()) {
        Time::Update();
        Input::BeginFrame();

        window.PollEvents();

        const float dt =
            Time::DeltaTime();

        // ====================================================
        // Local Movement
        // ====================================================

        if (
            Input::IsKeyDown(
                SDLK_W
            )
        ) {
            y -=
                speed *
                dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_S
            )
        ) {
            y +=
                speed *
                dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_A
            )
        ) {
            x -=
                speed *
                dt;
        }

        if (
            Input::IsKeyDown(
                SDLK_D
            )
        ) {
            x +=
                speed *
                dt;
        }

        // ====================================================
        // Multiplayer position sync
        // ====================================================

        sendTimer +=
            dt;

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

        // ====================================================
        // Render
        // ====================================================

        window.Clear(
            24,
            24,
            29,
            255
        );

        layers.ForEachOrdered(
            [&](Layer& layer) {

                // ============================================
                // Background
                // ============================================

                if (
                    layer.GetName() ==
                    "Background"
                ) {
                    /*
                     * Background rendering later.
                     */
                }

                // ============================================
                // World
                // ============================================

                else if (
                    layer.GetName() ==
                    "World"
                ) {
                    /*
                     * World / tilemap rendering later.
                     */
                }

                // ============================================
                // Entities
                // ============================================

                else if (
                    layer.GetName() ==
                    "Entities"
                ) {
                    std::vector<
                        PlayerRenderEntry
                    > renderPlayers;

                    // ========================================
                    // Local player
                    // ========================================

                    const int localID =
                        network.GetPlayerID();

                    PlayerRenderEntry local;

                    local.id =
                        localID;

                    local.name =
                        playerName;

                    local.x =
                        x;

                    local.y =
                        y;

                    local.local =
                        true;

                    local.color =
                        GetPlayerColor(
                            localID
                        );

                    renderPlayers.push_back(
                        local
                    );

                    // ========================================
                    // Remote players
                    // ========================================

                    const auto remotePlayers =
                        network.GetPlayers();

                    for (
                        const auto& [
                            id,
                            player
                        ] :
                        remotePlayers
                    ) {
                        /*
                         * Server broadcasts our own player
                         * back to us too.
                         *
                         * Don't draw ourselves twice.
                         */
                        if (
                            id ==
                            localID
                        ) {
                            continue;
                        }

                        PlayerRenderEntry remote;

                        remote.id =
                            id;

                        remote.name =
                            player.name;

                        remote.x =
                            player.x;

                        remote.y =
                            player.y;

                        remote.local =
                            false;

                        remote.color =
                            GetPlayerColor(
                                id
                            );

                        renderPlayers.push_back(
                            remote
                        );
                    }

                    // ========================================
                    // Automatic Player Layering
                    //
                    // Lower Y = behind
                    // Higher Y = in front
                    //
                    // This is basically a tiny automatic
                    // per-player render layer system.
                    // ========================================

                    std::stable_sort(
                        renderPlayers.begin(),
                        renderPlayers.end(),
                        [](
                            const PlayerRenderEntry& a,
                            const PlayerRenderEntry& b
                        ) {
                            if (
                                a.y ==
                                b.y
                            ) {
                                /*
                                 * Stable deterministic
                                 * tie breaker.
                                 */
                                return
                                    a.id <
                                    b.id;
                            }

                            return
                                a.y <
                                b.y;
                        }
                    );

                    // ========================================
                    // Render sorted players
                    // ========================================

                    for (
                        const PlayerRenderEntry& player :
                        renderPlayers
                    ) {
                        if (
                            !playerTexture.IsValid()
                        ) {
                            continue;
                        }

                        SDL_SetTextureColorMod(
                            playerTexture
                                .GetNativeTexture(),

                            player.color.r,
                            player.color.g,
                            player.color.b
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

                    // ========================================
                    // Reset texture tint
                    // ========================================

                    if (
                        playerTexture.IsValid()
                    ) {
                        SDL_SetTextureColorMod(
                            playerTexture
                                .GetNativeTexture(),
                            255,
                            255,
                            255
                        );
                    }
                }

                // ============================================
                // UI
                // ============================================

                else if (
                    layer.GetName() ==
                    "UI"
                ) {
                    /*
                     * Player names, HUD, chat, etc.
                     * will eventually render here.
                     */
                }
            }
        );

        window.Present();
    }

    network.Disconnect();

    return 0;
}

}
