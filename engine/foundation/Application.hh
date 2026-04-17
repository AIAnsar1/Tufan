#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <string>
#include <functional>

#include "IApplication.hh"
#include "Container.hh"
#include "../configuration/Repository.hh"
#include "../configuration/EnvParser.hh"
#include "ServiceRegistry.hh"
#include "../logger/Logger.hh"



namespace Tufan::Services
{
    class ServiceManager;
    class ServiceProvider;
}


namespace Tufan::Foundation
{
    class Application final : public IApplication
    {
        public:
            [[nodiscard]]
            static std::shared_ptr<Application>create(std::filesystem::path basePath = std::filesystem::current_path());

            explicit Application(std::filesystem::path basePath);
            ~Application() override;

            Application(const Application&) = delete;
            Application& operator=(const Application&) = delete;
            void bootstrap() override;
            void run() override;
            void terminate() override;

            [[nodiscard]]
            std::string_view name() const noexcept override;
            [[nodiscard]]
            std::string_view version() const noexcept override;
            [[nodiscard]]
            std::string_view environment() const noexcept override;
            [[nodiscard]]
            std::filesystem::path basePath() const noexcept override;

            [[nodiscard]]
            bool isBooted() const noexcept override;
            [[nodiscard]]
            bool isRunning() const noexcept override;
            [[nodiscard]]
            bool isDebug() const noexcept override;
            [[nodiscard]]
            bool isTesting() const noexcept override;
            [[nodiscard]]
            bool isProduction() const noexcept override;

            [[nodiscard]]
            Foundation::Container& container() noexcept;
            [[nodiscard]]
            Container::ModuleRegistry& registry() noexcept;
            [[nodiscard]]
            Configuration::Repository&  config() noexcept;
            [[nodiscard]]
            Logger::ILogger& logger() noexcept;
            [[nodiscard]]
            Services::ServiceManager&   services() noexcept;

            template<typename Provider, typename... Args> Application& register_(Args&&... args);

            Application& onBooting(std::function<void()> callback);
            Application& onBooted(std::function<void()>  callback);

            [[nodiscard]]
            std::filesystem::path path(std::string_view relative) const;

            [[nodiscard]]
            static std::string_view frameworkVersion() noexcept;

        private:
            void loadEnvironment();
            void loadConfiguration();
            void registerBaseBindings();
            void setupLogger();

            std::filesystem::path basePath_;
            std::string name_
            {
                "CyberForge App"
            };
            std::string version_
            {
                "0.1.0"
            };
            std::string environment_
            {
                "production"
            };

            bool booted_
            {
                false
            };
            bool running_
            {
                false
            };
            bool debug_
            {
                false
            };

            Container::Container container_;
            Container::ModuleRegistry registry_;
            Configuration::Repository config_;
            std::unique_ptr<Logger::Logger> logger_;
            std::unique_ptr<Services::ServiceManager> serviceManager_;
            std::vector<std::function<void()>> bootingCallbacks_;
            std::vector<std::function<void()>> bootedCallbacks_;
        };

    template<typename Provider, typename... Args> Application& Application::register_(Args&&... args)
    {
        services().add<Provider>(std::forward<Args>(args)...);
        return *this;
    }
}

//
// // Examples
// template<typename Provider, typename... Args> Application& Application::provide(Args&&... args)
// {
//     static_assert(std::is_base_of_v<Services::IServiceProvider, Provider>,"Provider must inherit from IServiceProvider");
//     providers_.emplace_back(std::make_unique<Provider>(*this, std::forward<Args>(args)...));
//     return *this;
// }