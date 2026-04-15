#include <print>
#include <filesystem>
#include <format>

#include "Application.hh"
#include "IProvider.hh"


namespace Tufan::Foundation
{
    std::shared_ptr<Application> Application::create(std::filesystem::path basePath) {
        return std::make_shared<Application>(std::move(basePath));
    }

    Application::Application(std::filesystem::path basePath): basePath_(std::move(basePath))
    {
        container_.instance<IApplication>(std::shared_ptr<IApplication>(this, [](IApplication*){}));
    }

    Application::~Application()
    {
        if (running_)
        {
            terminate();
        }
    }

    void Application::bootstrap()
    {
        if (booted_)
        {
            return;
        }
        loadEnvironment();
        loadConfiguration();
        registerBaseBindings();

        for (auto& cb : bootingCallbacks_)
        {
            cb();
        }
        registerProviders();
        bootProviders();

        for (auto& cb : bootedCallbacks_)
        {
            cb();
        }
        booted_ = true;
        std::println("[CyberForge] {} v{} booted in '{}' environment",name_, version_, environment_);
    }

    void Application::run()
    {
        if (!booted_)
        {
            bootstrap();
        }
        running_ = true;
    }

    void Application::terminate()
    {
        running_ = false;
        std::println("[CyberForge] Application terminated.");
    }


    std::string_view Application::name() const noexcept
    {
        return name_;
    }
    std::string_view Application::version() const noexcept
    {
        return version_;
    }
    std::string_view Application::environment() const noexcept
    {
        return environment_;
    }
    std::filesystem::path Application::basePath() const noexcept
    {
        return basePath_;
    }
    bool Application::isBooted() const noexcept
    {
        return booted_;
    }
    bool Application::isRunning() const noexcept
    {
        return running_;
    }
    bool Application::isDebug() const noexcept
    {
        return debug_;
    }
    bool Application::isTesting() const noexcept
    {
        return environment_ == "testing";
    }
    bool Application::isProduction() const noexcept
    {
        return environment_ == "production";
    }
    std::string_view Application::frameworkVersion() noexcept
    {
        return "0.1.0";
    }


    Container::Container& Application::container() noexcept
    {
        return container_;
    }
    Configuration::Repository& Application::config() noexcept
    {
        return config_;
    }

    Application& Application::onBooting(std::function<void()> callback)
    {
        bootingCallbacks_.push_back(std::move(callback));
        return *this;
    }

    Application& Application::onBooted(std::function<void()> callback)
    {
        bootedCallbacks_.push_back(std::move(callback));
        return *this;
    }

    std::filesystem::path Application::path(std::string_view relative) const
    {
        return basePath_ / relative;
    }


    void Application::loadEnvironment()
    {
        // Выбираем .env файл
        auto envFile = basePath_ / ".env";

        // Если APP_ENV=testing → .env.testing
        if (const char* appEnv = std::getenv("APP_ENV"))
        {
            environment_ = appEnv;
            auto envTesting = basePath_ / std::format(".env.{}", environment_);

            if (std::filesystem::exists(envTesting))
            {
                envFile = envTesting;
            }
        }

        if (!std::filesystem::exists(envFile))
        {
            std::println("[CyberForge] Warning: no .env file found at {}", envFile.string());
            return;
        }
        auto env = Configuration::EnvParser::parse(envFile);
        config_.setMany(env);

        // Применяем ключевые переменные
        if (config_.has("APP_NAME"))
        {
            name_ = config_.get<std::string>("APP_NAME");
        }

        if (config_.has("APP_VERSION"))
        {
            version_ = config_.get<std::string>("APP_VERSION");
        }

        if (config_.has("APP_ENV"))
        {
            environment_ = config_.get<std::string>("APP_ENV");
        }

        if (config_.has("APP_DEBUG"))
        {
            debug_ = config_.get<bool>("APP_DEBUG");
        }
    }

    void Application::loadConfiguration()
    {
        auto configDir = basePath_ / "config";
        if (!std::filesystem::exists(configDir))
        {
            return;
        }
    }

    void Application::registerBaseBindings()
    {
        container_.instance<Container::Container>(std::shared_ptr<Container::Container>(&container_, [](auto*){}));
    }

    void Application::registerProviders()
    {
        for (auto& provider : providers_)
        {
            provider->register_();
        }
    }

    void Application::bootProviders()
    {
        for (auto& provider : providers_)
        {
            provider->boot();
        }
    }
}