#pragma once

#include <source_location>
#include <format>

#include "../logger/Logger.hh"


namespace Tufan::Facades
{


    class Logger
    {
        public:
            Logger() = delete;

            static void setLogger(Tufan::Logger::ILogger* logger) noexcept
            {
                logger_ = logger;
            }

            template<typename... Args> static void trace(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Logger::LogLevel::Trace,std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> static void debug(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Logger::LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args> static void info(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Logger::LogLevel::Info,std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args>static void warning(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Logger::LogLevel::Warning,std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args>static void error(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Logger::LogLevel::Error,std::format(fmt, std::forward<Args>(args)...), loc);
            }

            template<typename... Args>static void critical(std::format_string<Args...> fmt, Args&&... args,std::source_location loc = std::source_location::current())
            {
                get().log(Tufan::Logger::Logger::LogLevel::Critical,std::format(fmt, std::forward<Args>(args)...), loc);
            }

        private:
            inline static Tufan::Logger::Logger::ILogger* logger_ = nullptr;

            [[nodiscard]]
            static Tufan::Logger::Logger::ILogger& get()
            {
                if (!logger_)
                {
                    throw std::runtime_error("[Log Facade] Logger not initialized");
                }
                return *logger_;
            }
        };

}