#pragma once

#include <string>
#include <string_view>
#include <any>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <format>

namespace Tufan::Configuration
{
    class Repository
    {
        public:
            Repository()  = default;
            ~Repository() = default;
            Repository(const Repository&) = delete;
            Repository& operator=(const Repository&) = delete;

            template<typename T> void set(std::string_view key, T&& value)
            {
                std::unique_lock lock(mutex_);
                data_[std::string(key)] = std::forward<T>(value);
            }

            void setMany(const std::unordered_map<std::string, std::string>& values)
            {
                std::unique_lock lock(mutex_);

                for (const auto& [k, v] : values)
                {
                    data_[k] = v;
                }
            }

            template<typename T> [[nodiscard]] T get(std::string_view key) const
            {
                std::shared_lock lock(mutex_);
                auto it = data_.find(std::string(key));

                if (it == data_.end())
                {
                    throw Tufan::Exceptions::ConfigException(std::format("Config key not found: '{}'", key));
                }
                return castValue<T>(it->second, key);
            }

            template<typename T> [[nodiscard]] T get(std::string_view key, T defaultValue) const noexcept
            {
                try {
                    return get<T>(key);
                } catch (...) {
                    return defaultValue;
                }
            }

            template<typename T> [[nodiscard]] std::optional<T> getOptional(std::string_view key) const noexcept
            {
                try {
                    return get<T>(key);
                } catch (...) {
                    return std::nullopt;
                }
            }

            [[nodiscard]]
            bool has(std::string_view key) const noexcept
            {
                std::shared_lock lock(mutex_);
                return data_.contains(std::string(key));
            }

            void remove(std::string_view key)
            {
                std::unique_lock lock(mutex_);
                data_.erase(std::string(key));
            }

            void clear()
            {
                std::unique_lock lock(mutex_);
                data_.clear();
            }

            [[nodiscard]]
            std::size_t size() const noexcept
            {
                std::shared_lock lock(mutex_);
                return data_.size();
            }

            [[nodiscard]]
            std::vector<std::string> keysWithPrefix(std::string_view prefix) const
            {
                std::shared_lock lock(mutex_);
                std::vector<std::string> result;

                for (const auto& [key, _] : data_)
                {
                    if (key.starts_with(prefix))
                    {
                        result.push_back(key);
                    }
                }
                return result;
            }

        private:
            mutable std::shared_mutex mutex_;
            std::unordered_map<std::string, std::any> data_;

            template<typename T> static T castValue(const std::any& value, std::string_view key)
            {
                if (value.type() == typeid(T))
                {
                    return std::any_cast<T>(value);
                }

                if (value.type() == typeid(std::string))
                {
                    const auto& str = std::any_cast<const std::string&>(value);
                    return fromString<T>(str, key);
                }

                throw Tufan::Exceptions::ConfigException(std::format("Type mismatch for key '{}': expected {}, got {}",key, typeid(T).name(), value.type().name()));
            }

            template<typename T> static T fromString(const std::string& str, std::string_view key)
            {
                if constexpr (std::is_same_v<T, std::string>)
                {
                    return str;
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    if (str == "true"  || str == "1" || str == "yes" || str == "on")
                    {
                        return true;
                    }

                    if (str == "false" || str == "0" || str == "no"  || str == "off")
                    {
                        return false;
                    }
                    throw Tufan::Exceptions::ConfigException(std::format("Cannot convert '{}' to bool for key '{}'", str, key));
                }
                else if constexpr (std::is_integral_v<T>)
                {
                    try {
                        return static_cast<T>(std::stoll(str));
                    }
                    catch (...) {
                        throw Tufan::Exceptions::ConfigException(std::format("Cannot convert '{}' to integer for key '{}'", str, key));
                    }
                }
                else if constexpr (std::is_floating_point_v<T>)
                {
                    try {
                        return static_cast<T>(std::stod(str));
                    }
                    catch (...) {
                        throw Tufan::Exceptions::ConfigException(std::format("Cannot convert '{}' to float for key '{}'", str, key));
                    }
                } else {
                    throw Tufan::Exceptions::ConfigException(std::format("Unsupported type conversion for key '{}'", key));
                }
            }
        };
}