#include "Time.hpp"

#include <SDL3/SDL.h>

namespace LibreGE {

std::uint64_t Time::s_LastCounter = 0;
std::uint64_t Time::s_FrameCount = 0;

float Time::s_DeltaTime = 0.0f;
float Time::s_FPS = 0.0f;

void Time::Init() {
    s_LastCounter =
        SDL_GetPerformanceCounter();

    s_FrameCount = 0;
    s_DeltaTime = 0.0f;
    s_FPS = 0.0f;
}

void Time::Update() {
    const std::uint64_t currentCounter =
        SDL_GetPerformanceCounter();

    const std::uint64_t frequency =
        SDL_GetPerformanceFrequency();

    if (
        s_LastCounter == 0 ||
        frequency == 0
    ) {
        s_LastCounter =
            currentCounter;

        s_DeltaTime = 0.0f;
        s_FPS = 0.0f;

        return;
    }

    const std::uint64_t elapsed =
        currentCounter -
        s_LastCounter;

    s_LastCounter =
        currentCounter;

    s_DeltaTime =
        static_cast<float>(
            static_cast<double>(elapsed) /
            static_cast<double>(frequency)
        );

    /*
     * Clamp huge frame jumps.
     *
     * Useful when:
     * - debugger pauses
     * - window is dragged
     * - system sleeps
     * - application stalls
     */
    constexpr float maxDelta =
        0.25f;

    if (s_DeltaTime > maxDelta) {
        s_DeltaTime = maxDelta;
    }

    if (s_DeltaTime > 0.0f) {
        s_FPS =
            1.0f /
            s_DeltaTime;
    }
    else {
        s_FPS = 0.0f;
    }

    ++s_FrameCount;
}

float Time::DeltaTime() {
    return s_DeltaTime;
}

float Time::FPS() {
    return s_FPS;
}

std::uint64_t Time::FrameCount() {
    return s_FrameCount;
}

}
