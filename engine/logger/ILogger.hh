#pragma once

#include <string>
#include <string_view>
#include <source_location>
#include <format>


namespace Tufan::Logger
{
    enum class LogLevel : uint8_t
    {
        Trace   = 0,
        Debug   = 1,
        Info    = 2,
        Warning = 3,
        Error   = 4,
        Critical= 5,
        Off     = 6
    };

    [[nodiscard]]
    constexpr std::string_view levelToString(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::Trace:
                return "TRACE";
            case LogLevel::Debug:
                return "DEBUG";
            case LogLevel::Info:
                return "INFO";
            case LogLevel::Warning:
                return "WARNING";
            case LogLevel::Error:
                return "ERROR";
            case LogLevel::Critical:
                return "CRITICAL";
            default:
                return "OFF";
        }
    }


    class ILogger
    {
        public:
            virtual ~ILogger() = default;
            virtual void log(LogLevel level,std::string_view message,std::source_location loc = std::source_location::current()) = 0;
            template<typename... Args> void trace(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Trace, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> void debug(std::format_string<Args...> fmt, Args&&... args, std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> void info(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> void warning(std::format_string<Args...> fmt, Args&&... args, std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> void error(std::format_string<Args...> fmt, Args&&... args, std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> void critical(std::format_string<Args...> fmt, Args&&... args, std::source_location loc = std::source_location::current())
            {
                log(LogLevel::Critical, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            virtual void setLevel(LogLevel level) noexcept = 0;
            [[nodiscard]] virtual LogLevel getLevel() const noexcept = 0;
        };

}