#pragma once
#include <concepts>
#include <evo/core/types.h>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>

namespace print {
    enum class PrintSeverity : u8 { Debug, Info, Warn, Error, Fatal };

    // i fucking hate c++
    template <typename... T>
    struct PrintFormat {
        std::format_string<T...> format;
        std::source_location location;

        template <typename S>
            requires std::convertible_to<const S&, std::string_view>
        consteval PrintFormat(const S& s, std::source_location loc = std::source_location::current()) : format(s), location(loc) {
        }
    };

    void PrintImplementation(PrintSeverity severity, std::string_view message, std::source_location location);

    template <typename... T>
    void Print(PrintSeverity severity, PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
        std::string formatted_message = std::format(message.format, std::forward<T>(args)...);
        PrintImplementation(severity, formatted_message, message.location);
    }

    template <typename... T>
    void Debug(PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
#ifdef EVO_DEBUG
        Log(PrintSeverity::Debug, message, std::forward<T>(args)...);
#endif
    }

    template <typename... T>
    void Info(PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
        Print(PrintSeverity::Info, message, std::forward<T>(args)...);
    }

    template <typename... T>
    void Warn(PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
        Print(PrintSeverity::Warn, message, std::forward<T>(args)...);
    }

    template <typename... T>
    void Error(PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
        Print(PrintSeverity::Error, message, std::forward<T>(args)...);
    }

    template <typename... T>
    void Fatal(PrintFormat<std::type_identity_t<T>...> message, T&&... args) {
        Print(PrintSeverity::Fatal, message, std::forward<T>(args)...);
    }
} // namespace print
