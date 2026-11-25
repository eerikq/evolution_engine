#include <evo/core/time.h>
#include <time.h>

static u64 GetCurrentTick() {
    struct timespec time_spec;
    timespec_get(&time_spec, TIME_UTC);
    return (u64)(time_spec.tv_sec * 1000000000.0 + time_spec.tv_nsec);
}

inline static f64 TickToSeconds(const u64 tick) {
    return (f64)tick / 1000000000.0;
};

void core::UpdateTime(Time* time) {
    u64 current_tick = GetCurrentTick();

    // first time initialization
    if (time->start_tick == 0) {
        time->start_tick = current_tick;
        time->prev_tick = current_tick;
    }

    time->time = TickToSeconds(current_tick - time->start_tick);
    time->delta_time = (f32)TickToSeconds(current_tick - time->prev_tick);
    time->fps = time->delta_time > 0.0f ? 1.0f / time->delta_time : 0.0f;
    time->prev_tick = current_tick;
    time->frame_count += 1;

    // smooth fps calculations
    time->fps_accum_frames++;
    time->fps_accum_time += time->delta_time;
    if (time->fps_accum_time > 1.0f) {
        time->fps_smooth = (f32)time->fps_accum_frames / time->fps_accum_time;

        time->fps_accum_time = 0.0f;
        time->fps_accum_frames = 0;
    }
}

core::WallClock core::GetWallClock(bool is_local) {
    struct timespec time_spec;
    timespec_get(&time_spec, TIME_UTC);

    tm* time = is_local ? localtime(&time_spec.tv_sec) : gmtime(&time_spec.tv_sec);

    return {
        .hours = (u16)(time->tm_hour),
        .minutes = (u16)(time->tm_min),
        .seconds = (u16)(time->tm_sec),
        .milliseconds = (u16)(time_spec.tv_nsec / 1000000),
    };
}
