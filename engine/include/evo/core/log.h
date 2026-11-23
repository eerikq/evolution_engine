#pragma once

enum class PrintSeverity { Debug, Info, Warn, Error, Fatal };

namespace core {
    // Example Log:
    // [03:18:23][ERROR] This is an error message.
    void Log(PrintSeverity severity, const char* message, ...);
} // namespace core
