#pragma once
#include <evo/core/types.h>

namespace core {
    struct WallClock {
        u16 hours = 0;
        u16 minutes = 0;
        u16 seconds = 0;
        u16 milliseconds = 0;
    };

    struct Time {
        u64 start_tick = 0; // stored in nanoseconds
        u64 prev_tick = 0;  // stored in nanoseconds
        f32 fps_accum_time = 0.0f;
        u32 fps_accum_frames = 0;
        // ↑↑↑ internal variables to keep time stateless
        f64 time = 0.0;        // elapsed time since the start in seconds
        f32 delta_time = 0.0f; // time difference between the last frame and the current one in seconds
        f32 fps = 0.0f;        // current frames per second compared to the last frame
        f32 fps_smooth = 0.0f; // current frames per second compared to the average of the previous second
        u32 frame_count = 0;   // total number of frames since the start
    };

    void UpdateTime(Time* time);
    WallClock GetWallClock(bool is_local = true);
} // namespace core
