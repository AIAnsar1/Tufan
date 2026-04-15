#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <filesystem>


namespace Tufan::Foundation
{
    class IApplication
    {
        public:
            virtual ~IApplication() = default;
            virtual void bootstrap() = 0;
            virtual void run() = 0;
            virtual void terminate() = 0;

            [[nodiscard]]
            virtual std::string_view name() const noexcept = 0;
            [[nodiscard]]
            virtual std::string_view version() const noexcept = 0;
            [[nodiscard]]
            virtual std::string_view environment() const noexcept = 0;
            [[nodiscard]]
            virtual std::filesystem::path basePath() const noexcept = 0;
            [[nodiscard]]
            virtual bool isBooted() const noexcept = 0;
            virtual bool isRunning() const noexcept = 0;
            virtual bool isDebug() const noexcept = 0;
            virtual bool isTesting() const noexcept = 0;
            virtual bool isProduction() const noexcept = 0;
    };
}




























