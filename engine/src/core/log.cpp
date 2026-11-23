#pragma once

#include "evo/core/time.h"

#include <evo/core/log.h>
#include <fmt/color.h>

void core::Log(PrintSeverity severity, const char* message, ...) {
#ifdef EVO_DEBUG
    fmt::color severity_color = fmt::color::mint_cream;
    const char* severity_text = "NONE!";

    switch (severity) {
        case PrintSeverity::Debug:
            severity_color = fmt::color::medium_aquamarine;
            severity_text = "Debug";
            break;
        case PrintSeverity::Info:
            severity_color = fmt::color::mint_cream;
            severity_text = "Info.";
            break;
        case PrintSeverity::Warn:
            severity_color = fmt::color::gold;
            severity_text = "Warn.";
            break;
        case PrintSeverity::Error:
            severity_color = fmt::color::red;
            severity_text = "Error";
            break;
        case PrintSeverity::Fatal:
            severity_color = fmt::color::maroon;
            severity_text = "Fatal";
            break;
    }

    fmt::memory_buffer buffer;

    WallClock time = GetWallClock();

    // using this directly doesn't account for timezones. e.g. "20:24" gets printed as "18:24"
    std::string message_string = fmt::format(fg(fmt::color::dim_gray), "[{}:{}:{}:{}]", time.hours, time.minutes, time.milliseconds, time.milliseconds);
    // fg(severity_color), "[{}] ", severity_text),
    // fg(severity_color), time.hours, time.minutes, time.milliseconds, time.milliseconds,

    fmt::println("{}", fmt::string_view(buffer.data(), buffer.size()));
#endif
}
