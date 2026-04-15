#pragma once

#include <vector>
#include <memory>
#include <filesystem>
#include <string>

#include "IApplication.hh"
#include "Container.hh"
#include "../configuration/Repository.hh"
#include "../configuration/EnvParser.hh"


namespace Tufan::Foundation
{
    class Application final : public IApplication
    {
    public:
        [[nodiscard]]
        static std::shared_ptr<Application> create(std::filesystem::path basePath = std::filesystem::current_path());

        explicit Application(std::filesystem::path basePath);
        ~Application() override;

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        void bootstrap() override;
        void run() override;
        void terminate() override;

        [[nodiscard]]
        std::string_view  name() const noexcept override;
        [[nodiscard]]
        std::string_view  version() const noexcept override;
        [[nodiscard]]
        std::string_view  environment() const noexcept override;
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
        Container::Container& container() noexcept;

        [[nodiscard]]
        Configuration::Repository& config() noexcept;

        template<typename Provider, typename... Args> Application& provide(Args&&... args);
        Application& onBooting(std::function<void()> callback);
        Application& onBooted(std::function<void()> callback);

        [[nodiscard]]
        std::filesystem::path path(std::string_view relative) const;

        [[nodiscard]]
        static std::string_view frameworkVersion() noexcept;

    private:
        void loadEnvironment();
        void loadConfiguration();
        void registerBaseBindings();
        void registerProviders();
        void bootProviders();

        std::filesystem::path basePath_;
        std::string name_ { "CyberForge App" };
        std::string version_ { "0.1.0" };
        std::string environment_ { "production" };
        bool booted_ { false };
        bool running_ { false };
        bool debug_ { false };
        Container::Container container_;
        Configuration::Repository config_;
        std::vector<std::unique_ptr<Services::IServiceProvider>> providers_;
        std::vector<std::function<void()>> bootingCallbacks_;
        std::vector<std::function<void()>> bootedCallbacks_;
    };
}

//
// // Examples
// template<typename Provider, typename... Args> Application& Application::provide(Args&&... args)
// {
//     static_assert(std::is_base_of_v<Services::IServiceProvider, Provider>,"Provider must inherit from IServiceProvider");
//     providers_.emplace_back(std::make_unique<Provider>(*this, std::forward<Args>(args)...));
//     return *this;
// }