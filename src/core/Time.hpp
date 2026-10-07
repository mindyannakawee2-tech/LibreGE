#pragma once

#include <cstdint>

namespace LibreGE {

class Time {
public:
    static void Init();
    static void Update();

    static float DeltaTime();
    static float FPS();

    static std::uint64_t FrameCount();

private:
    static std::uint64_t s_LastCounter;
    static std::uint64_t s_FrameCount;

    static float s_DeltaTime;
    static float s_FPS;
};

}
