#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <functional>
#include <concepts>

namespace Tufan::Foundation
{
    class Application;
    class ServiceRegistry;

}
namespace Tufan::Configuration
{
    class Repository;
}

namespace Tufan::Logger
{
    class ILogger;
}

namespace Tufan::Services
{
    template<typename T> concept IsServiceProvider = requires(T t)
    {
            { t.register_() } -> std::same_as<void>;
            { t.boot()      } -> std::same_as<void>;
    };

    class ServiceProvider
    {
        public:
            explicit ServiceProvider(Foundation::Application& app);
            virtual ~ServiceProvider() = default;

            ServiceProvider(const ServiceProvider&) = delete;
            ServiceProvider& operator=(const ServiceProvider&) = delete;

            [[nodiscard]]
            virtual std::string_view name() const noexcept = 0;

            virtual void register_() = 0;

            virtual void boot() {}

            [[nodiscard]]
            virtual std::vector<std::string> provides() const noexcept
            {
                return {};
            }


            [[nodiscard]]
            virtual bool isDeferred() const noexcept
            {
                return false;
            }


            [[nodiscard]]
            virtual std::vector<std::string> dependencies() const noexcept
            {
                return {};
            }

            [[nodiscard]]
            bool isRegistered() const noexcept
            {
                return registered_;
            }

            [[nodiscard]]
            bool isBooted() const noexcept
            {
                return booted_;
            }

        protected:
            [[nodiscard]]
            Foundation::Application& app() noexcept;
            [[nodiscard]]
            Foundation::ServiceRegistry& registry() noexcept;
            [[nodiscard]]
            Configuration::Repository& config() noexcept;
            [[nodiscard]]
            Logger::ILogger& logger() noexcept;

            void callAfterBoot(std::function<void()> callback);

        private:
            friend class ServiceManager;

            void markRegistered() noexcept
            {
                registered_ = true;
            }

            void markBooted()     noexcept
            {
                booted_     = true;
            }

            Foundation::Application& app_;

            bool registered_
            {
                false
            };

            bool booted_
            {
                false
            };
        };
}



























