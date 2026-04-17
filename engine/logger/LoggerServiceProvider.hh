#pragma once

#include "../services/ServiceProvider.hh"
#include "Logger.hh"

namespace Tufan::Logger
{

    class LogServiceProvider final : public ServiceProvider
    {
        public:
            using ServiceProvider::ServiceProvider;

            [[nodiscard]]
            std::string_view name() const noexcept override
            {
                return "LogServiceProvider";
            }

            void register_() override
            {
                auto levelStr = config().get<std::string>("LOG_LEVEL","info");
                auto channel = config().get<std::string>("LOG_CHANNEL","console");
                auto filePath  = config().get<std::string>("LOG_FILE","./logs/app.log");
                Logger::LogLevel level = Logger::LogLevel::Info;

                if  (levelStr == "trace")
                {
                    level = Logger::LogLevel::Trace;
                }
                else if (levelStr == "debug")
                {
                    level = Logger::LogLevel::Debug;
                }
                else if (levelStr == "info")
                {
                    level = Logger::LogLevel::Info;
                }
                else if (levelStr == "warning")
                {
                    level = Logger::LogLevel::Warning;
                }
                else if (levelStr == "error")
                {
                    level = Logger::LogLevel::Error;
                }
                else if (levelStr == "critical")
                {
                    level = Logger::LogLevel::Critical;
                }

                Logger::LoggerConfig cfg
                {
                    .level = level,
                    .toConsole = (channel == "console" || channel == "both"),
                    .toFile = (channel == "file"    || channel == "both"),
                    .filePath = filePath,
                    .colorized = app().isDebug() || !app().isProduction()
                };
                Logger::Global::init(cfg);
            }

            void boot() override
            {
                logger().info("Logger initialized [level={}, env={}]",config().get<std::string>("LOG_LEVEL", "info"),std::string(app().environment()));
            }
    };

}