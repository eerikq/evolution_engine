#include <chrono>
#include <evo/core/time.h>

void core::UpdateTime(Time* time) {
    time->frame_count += 1;
    time->time = 0.0;
    time->delta_time = 0.0f;
    time->fps = 1.0f / time->delta_time;
    time->fps_smooth = 0.0f;

    time->last_time = 0.0;
    time->fps_accum_time = 0.0f;
    time->fps_accum_frames = 0.0f;
}

core::WallClock GetWallClock(bool is_local = true) {
    auto now = std::chrono::system_clock::now();

    if (is_local) {
        auto local_now = std::chrono::current_zone()->to_local(now);
        auto duration = local_now.time_since_epoch();

        return {
            .hours = static_cast<u32>(std::chrono::duration_cast<std::chrono::hours>(duration).count()),
            .minutes = static_cast<u32>(std::chrono::duration_cast<std::chrono::minutes>(duration).count()),
            .seconds = static_cast<u32>(std::chrono::duration_cast<std::chrono::seconds>(duration).count()),
            .milliseconds = static_cast<u32>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()),
        };
    }

    auto duration = now.time_since_epoch();
    return {
        .hours = static_cast<u32>(std::chrono::duration_cast<std::chrono::hours>(duration).count()),
        .minutes = static_cast<u32>(std::chrono::duration_cast<std::chrono::minutes>(duration).count()),
        .seconds = static_cast<u32>(std::chrono::duration_cast<std::chrono::seconds>(duration).count()),
        .milliseconds = static_cast<u32>(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()),
    };
}
