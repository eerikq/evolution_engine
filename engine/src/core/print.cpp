#include <cstdio>
#include <evo/core/print.h>
#include <evo/core/time.h>
#include <iterator>
#include <string>

void print::PrintImplementation(PrintSeverity severity, std::string_view message, std::source_location location) {
    constexpr const char* COLOR_RESET = "\033[0m";
    constexpr const char* COLOR_GRAY = "\033[38;2;128;128;128m";  // rgb(128, 128, 128)
    constexpr const char* COLOR_DEBUG = "\033[38;2;102;205;170m"; // rgb(102, 205, 170)
    constexpr const char* COLOR_INFO = "\033[38;2;245;255;250m";  // rgb(245, 255, 250)
    constexpr const char* COLOR_WARN = "\033[38;2;255;215;0m";    // rgb(255, 215, 0)
    constexpr const char* COLOR_ERROR = "\033[38;2;255;0;0m";     // rgb(255, 0, 0)
    constexpr const char* COLOR_FATAL = "\033[38;2;128;0;0m";     // rgb(128, 0, 0)

    const char* severity_text = "NONE!";
    const char* severity_color = COLOR_INFO;

    switch (severity) {
        case PrintSeverity::Debug:
            severity_color = COLOR_DEBUG;
            severity_text = "Debug";
            break;
        case PrintSeverity::Info:
            severity_color = COLOR_INFO;
            severity_text = "Info.";
            break;
        case PrintSeverity::Warn:
            severity_color = COLOR_WARN;
            severity_text = "Warn.";
            break;
        case PrintSeverity::Error:
            severity_color = COLOR_ERROR;
            severity_text = "Error";
            break;
        case PrintSeverity::Fatal:
            severity_color = COLOR_FATAL;
            severity_text = "Fatal";
            break;
    }

    std::string buffer;
    core::WallClock time = core::GetWallClock();

    std::format_to(std::back_inserter(buffer), "{}[{:02}:{:02}:{:02}.{:03}]", COLOR_GRAY, time.hours, time.minutes, time.seconds, time.milliseconds);
    std::format_to(std::back_inserter(buffer), "{}[{}]: ", severity_color, severity_text);
    std::format_to(std::back_inserter(buffer), "{}", message);
    std::format_to(std::back_inserter(buffer), "{} @ {}:{}:{}", COLOR_GRAY, location.file_name(), location.line(), location.column());
    std::format_to(std::back_inserter(buffer), "{}\n", COLOR_RESET);

    std::fwrite(buffer.data(), 1, buffer.size(), stdout);
}
