#pragma once

#include <memory>
#include <stdexcept>

#include "../foundation/Application.hh"


namespace Tufan::Facades
{

    template<typename Service> class Facade
    {
    public:
        Facade() = delete;  // purely static
        virtual ~Facade() = default;

        static void setApp(Foundation::Application* app) noexcept
        {
            app_ = app;
        }

    protected:
        [[nodiscard]]
        static Service& root()
        {
            if (!app_)
            {
                throw std::runtime_error("[Facade] Application not set. Call Facade::setApp() first.");
            }
            return getRoot(*app_);
        }

        [[nodiscard]]
        virtual Service& getRoot(Foundation::Application& app) = 0;

    private:
        inline static Foundation::Application* app_ = nullptr;
    };

}