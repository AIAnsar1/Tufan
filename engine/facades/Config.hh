#pragma once


#include <string_view>
#include <optional>
#include <stdexcept>

#include "../configuration/Repository.hh"
#include "../foundation/Application.hh"


namespace Tufan::Facades {


    class Config
    {
        public:
            Config() = delete;

            static void setApp(Foundation::Application* app) noexcept
            {
                app_ = app;
            }

            template<typename T> [[nodiscard]] static T get(std::string_view key)
            {
                return repo().get<T>(key);
            }

            template<typename T> [[nodiscard]] static T get(std::string_view key, T defaultValue) noexcept
            {
                return repo().get<T>(key, std::move(defaultValue));
            }

            template<typename T> [[nodiscard]] static std::optional<T> optional(std::string_view key) noexcept
            {
                return repo().getOptional<T>(key);
            }

            [[nodiscard]]
            static bool has(std::string_view key) noexcept
            {
                return repo().has(key);
            }

            template<typename T>static void set(std::string_view key, T&& value)
            {
                repo().set(key, std::forward<T>(value));
            }

        private:
            inline static Foundation::Application* app_ = nullptr;

            [[nodiscard]]
            static Configuration::Repository& repo()
            {
                if (!app_)
                {
                    throw std::runtime_error("[Config Facade] App not initialized");
                }
                return app_->config();
            }
    };

}