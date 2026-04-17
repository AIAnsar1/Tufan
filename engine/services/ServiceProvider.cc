#include "ServiceProvider.hh"
#include "../foundation/Application.hh"
#include "../foundation/ServiceRegistry.hh"
#include "../configuration/Repository.hh"
#include "../logger/ILogger.hh"

namespace Tufan::Services
{

    ServiceProvider::ServiceProvider(Foundation::Application& app): app_(app)
    {

    }

    Foundation::Application& ServiceProvider::app() noexcept
    {
        return app_;
    }

    Foundation::ServiceRegistry& ServiceProvider::registry() noexcept
    {
        return app_.registry();
    }

    Configuration::Repository& ServiceProvider::config() noexcept
    {
        return app_.config();
    }

    Logger::ILogger& ServiceProvider::logger() noexcept
    {
        return app_.logger();
    }

    void ServiceProvider::callAfterBoot(std::function<void()> callback)
    {
        app_.onBooted(std::move(callback));
    }

}