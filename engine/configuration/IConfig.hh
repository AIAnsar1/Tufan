#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <vector>
#include <stdexcept>


namespace Tufan::Configuration
{
    class IConfig
    {
        public:
            virtual ~IConfig() = default;
            template<typename T> [[nodiscard]] T get(std::string_view key) const;
            template<typename T> [[nodiscard]] T get(std::string_view key, T default_value) const noexcept;
            template<typename T> [[nodiscard]] std::optional<T> getOptional(std::string_view key) const noexcept;
            template<typename T> [[nodiscard]] std::vector<T> getArray(std::string_view key) const;

            [[nodiscard]]
            virtual bool has(std::string_view key) const noexcept = 0;

            template<typename T> void set(std::string_view key, T&& value);
            virtual void load(const std::string& filePath) = 0;
            virtual void loadEnv(const std::string& envPath) = 0;
            virtual void reload()                             = 0;

        protected:
            [[nodiscard]]
            virtual std::string getRaw(std::string_view key) const = 0;
            [[nodiscard]]
            virtual bool hasRaw(std::string_view key) const noexcept = 0;
            virtual void setRaw(std::string_view key, std::string value) = 0;
    };
}