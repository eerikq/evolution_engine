#pragma once
#include <evo/core/types.h>

namespace core {
    struct WallClock {
        u32 hours = 0;
        u32 minutes = 0;
        u32 seconds = 0;
        u32 milliseconds = 0;
    };

    struct Time {
        u32 frame_count = 0;   // total number of frames since the start
        f64 time = 0.0;        // elapsed time since the start in seconds
        f32 delta_time = 0.0f; // time difference between the last frame and the current one in seconds
        f32 fps = 0.0f;        // current frames per second compared to the last frame
        f32 fps_smooth = 0.0f; // current frames per second compared to the average of the previous second

        // internal variables to keep time stateless
        f64 last_time = 0.0;
        f32 fps_accum_time = 0.0f;
        f32 fps_accum_frames = 0.0f;
    };

    void UpdateTime(Time* time);
    WallClock GetWallClock(bool is_local = true);
} // namespace core
