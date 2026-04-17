
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <filesystem>
#include <print>
#include <format>
#include <memory>
#include <mutex>

#include "Logger.hh"


namespace Tufan::Logger {


    namespace Color
    {
        constexpr std::string_view Reset   = "\033[0m";
        constexpr std::string_view Gray    = "\033[90m";
        constexpr std::string_view Cyan    = "\033[96m";
        constexpr std::string_view Green   = "\033[92m";
        constexpr std::string_view Yellow  = "\033[93m";
        constexpr std::string_view Red     = "\033[91m";
        constexpr std::string_view Magenta = "\033[95m";
        constexpr std::string_view Bold    = "\033[1m";
    }


    Logger::Logger(LoggerConfig config): config_(std::move(config)), currentLevel_(config_.level)
    {
        setup();
    }


    void Logger::setup()
    {
        logging::add_common_attributes();

        if (config_.toConsole)
        {
            setupConsole();
        }

        if (config_.toFile)
        {
            setupFile();
        }
        setLevel(currentLevel_);
    }

    void Logger::setupConsole()
    {
        logging::add_console_log(std::clog,keywords::format = (expr::stream << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S") << " " << expr::attr<logging::trivial::severity_level>("Severity") << " " << expr::smessage));
    }

    void Logger::setupFile()
    {
        std::filesystem::create_directories(config_.filePath.parent_path());

        logging::add_file_log(
            keywords::file_name = config_.filePath.string() + ".%N",
            keywords::rotation_size = config_.maxFileSize,
            keywords::max_files = config_.maxFiles,
            keywords::auto_flush = true,
            keywords::open_mode = std::ios::app,
            keywords::format = (expr::stream << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f") << " [" << logging::trivial::severity << "]" << " [tid:" << expr::attr<attrs::current_thread_id::value_type>("ThreadID") << "]" << " " << expr::smessage));
    }


    void Logger::log(LogLevel level,std::string_view message,std::source_location loc)
    {
        if (level < currentLevel_)
        {
            return;
        }
        const auto color= config_.colorized ? levelColor(level) : "";
        const auto reset = config_.colorized ? Color::Reset : "";
        const auto locStr= formatLocation(loc);
        const auto levelStr = levelToString(level);

        if (config_.colorized && config_.toConsole)
        {
            std::print(stderr,"{}[{:^8}]{} {} {}{}{}\n",color, levelStr, reset,locStr,color, message, reset);
        }
        else
        {
            switch (level)
            {
                case LogLevel::Trace:
                    BOOST_LOG_TRIVIAL(trace) << message;
                    break;
                case LogLevel::Debug:
                    BOOST_LOG_TRIVIAL(debug) << message;
                    break;
                case LogLevel::Info:
                    BOOST_LOG_TRIVIAL(info) << message;
                    break;
                case LogLevel::Warning:
                    BOOST_LOG_TRIVIAL(warning) << message;
                    break;
                case LogLevel::Error:
                    BOOST_LOG_TRIVIAL(error) << message;
                    break;
                case LogLevel::Critical:
                    BOOST_LOG_TRIVIAL(fatal) << message;
                    break;
                default:
                    break;
            }
        }
    }


    void Logger::setLevel(LogLevel level) noexcept
    {
        currentLevel_ = level;
        logging::core::get()->set_filter(logging::trivial::severity >= toBoostLevel(level));
    }

    LogLevel Logger::getLevel() const noexcept
    {
        return currentLevel_;
    }

    std::shared_ptr<Logger> Logger::channel(std::string name) const
    {
        auto child = std::make_shared<Logger>(config_);
        child->channelName_ = std::move(name);
        child->currentLevel_ = currentLevel_;
        return child;
    }

    Logger::LevelGuard Logger::withLevel(LogLevel level) noexcept
    {
        LevelGuard guard { *this, currentLevel_ };
        setLevel(level);
        return guard;
    }

    void Logger::flush()
    {
        logging::core::get()->flush();
    }


    logging::trivial::severity_level Logger::toBoostLevel(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::Trace:
                return logging::trivial::trace;
            case LogLevel::Debug:
                return logging::trivial::debug;
            case LogLevel::Info:
                return logging::trivial::info;
            case LogLevel::Warning:
                return logging::trivial::warning;
            case LogLevel::Error:
                return logging::trivial::error;
            case LogLevel::Critical:
                return logging::trivial::fatal;
            default:
                return logging::trivial::info;
        }
    }

    std::string Logger::formatLocation(const std::source_location& loc)
    {
        std::filesystem::path p(loc.file_name());
        return std::format("{}:{}", p.filename().string(), loc.line());
    }

    std::string_view Logger::levelColor(LogLevel level) noexcept
    {
        switch (level)
        {
            case LogLevel::Trace:
                return Color::Gray;
            case LogLevel::Debug:
                return Color::Cyan;
            case LogLevel::Info:
                return Color::Green;
            case LogLevel::Warning:
                return Color::Yellow;
            case LogLevel::Error:
                return Color::Red;
            case LogLevel::Critical:
                return Color::Magenta;
            default:
                return Color::Reset;
        }
    }

    namespace Global
    {

        static std::unique_ptr<Logger> globalLogger;
        static std::once_flag           initFlag;

        void init(LoggerConfig config)
        {
            std::call_once(initFlag, [cfg = std::move(config)]() mutable
            {
                globalLogger = std::make_unique<Logger>(std::move(cfg));
            });
        }

        ILogger& get()
        {
            if (!globalLogger)
            {
                init();
            }
            return *globalLogger;
        }

    }

}















