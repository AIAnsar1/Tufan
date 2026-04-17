#pragma once

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/named_scope.hpp>

#include <string>
#include <string_view>
#include <filesystem>
#include <memory>
#include <source_location>

#include "ILogger.hh"


namespace Tufan::Logger
{
    namespace logging = boost::log;
    namespace sinks = boost::log::sinks;
    namespace keywords = boost::log::keywords;
    namespace expr = boost::log::expressions;
    namespace attrs = boost::log::attributes;

    struct LoggerConfig {
        LogLevel level { LogLevel::Info };
        bool toConsole { true  };
        bool toFile      { false };
        std::filesystem::path filePath { "../../storage/logs/tufan.log" };
        std::size_t maxFileSize { 10 * 1024 * 1024 };  // 10 MB
        std::size_t maxFiles { 5 };
        bool colorized { true  };
        std::string format { "[%TimeStamp%] [%Severity%] %Message%" };
    };

    class Logger final : public ILogger
    {
        public:
            explicit Logger(LoggerConfig config = {});
            ~Logger() override = default;

            void log(LogLevel level,std::string_view message,std::source_location loc = std::source_location::current()) override;
            void setLevel(LogLevel level) noexcept override;

            [[nodiscard]]
            LogLevel getLevel() const noexcept override;


            [[nodiscard]]
            std::shared_ptr<Logger> channel(std::string name) const;

            struct LevelGuard {
                Logger&   logger;
                LogLevel  prev;
                ~LevelGuard() { logger.setLevel(prev); }
            };

            [[nodiscard]]
            LevelGuard withLevel(LogLevel level) noexcept;

            void flush();

        private:
            void setup();
            void setupConsole();
            void setupFile();

            [[nodiscard]]
            static logging::trivial::severity_level toBoostLevel(LogLevel level) noexcept;

            [[nodiscard]]
            static std::string formatLocation(const std::source_location& loc);

            [[nodiscard]]
            static std::string_view levelColor(LogLevel level) noexcept;

            LoggerConfig config_;
            LogLevel currentLevel_;
            std::string channelName_ { "tufan" };
    };

    void init(LoggerConfig config = {});
    ILogger& get();

    template<typename... Args> void trace(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Trace, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args> void debug(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Debug, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args> void info(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Info, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args> void warning(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Warning, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args> void error(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Error, std::format(fmt, std::forward<Args>(args)...));
    }

    template<typename... Args> void critical(std::format_string<Args...> fmt, Args&&... args)
    {
        get().log(LogLevel::Critical, std::format(fmt, std::forward<Args>(args)...));
    }
}








































