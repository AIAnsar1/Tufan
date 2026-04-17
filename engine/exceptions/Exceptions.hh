#pragma once

#include <stdexcept>
#include <string>
#include <string_view>
#include <source_location>
#include <format>
#include <optional>
#include <vector>

namespace Tufan::Exceptions
{


    class CyberForgeException : public std::runtime_error
    {
    public:
        explicit CyberForgeException(std::string  message,std::source_location loc = std::source_location::current()): std::runtime_error(message), message_(std::move(message)), location_(loc)
        {

        }

        [[nodiscard]]
        std::string_view message() const noexcept
        {
            return message_;
        }

        [[nodiscard]]
        const std::source_location& location() const noexcept
        {
            return location_;
        }

        [[nodiscard]]
        std::string fullMessage() const
        {
            return std::format("[{}:{}] {}",std::string(location_.file_name()),location_.line(),message_);
        }

        [[nodiscard]]
        virtual std::string_view category() const noexcept
        {
            return "Tufan";
        }

        [[nodiscard]]
        virtual int exitCode() const noexcept
        {
            return 1;
        }

    protected:
        std::string message_;
        std::source_location location_;
    };


    class ConfigException : public CyberForgeException
    {
        public:
            using CyberForgeException::CyberForgeException;
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "Config";
            }
    };

    class EnvNotFoundException : public ConfigException
    {
        public:
            explicit EnvNotFoundException(std::string_view file,std::source_location loc = std::source_location::current()): ConfigException(std::format("Env file not found: '{}'", file), loc)
            {

            }
    };

    class KeyNotFoundException : public ConfigException
    {
        public:
            explicit KeyNotFoundException(std::string_view key,std::source_location loc = std::source_location::current()): ConfigException(std::format("Config key not found: '{}'", key), loc)
            {

            }
    };

    class RegistryException : public CyberForgeException
    {
        public:
            using CyberForgeException::CyberForgeException;
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "Registry";
            }
    };

    class ComponentNotFoundException : public RegistryException
    {
        public:
            explicit ComponentNotFoundException(std::string_view alias,std::source_location loc = std::source_location::current()): RegistryException(std::format("Component not found: '{}'", alias), loc)
            {

            }
    };

    class ComponentDisabledException : public RegistryException
    {
        public:
            explicit ComponentDisabledException(std::string_view alias,std::source_location loc = std::source_location::current()): RegistryException(std::format("Component is disabled: '{}'", alias), loc)
            {

            }
    };

    class DependencyException : public RegistryException
    {
        public:
            explicit DependencyException(std::string_view component,std::string_view dep,std::source_location loc = std::source_location::current()): RegistryException(std::format("'{}' depends on '{}' which is not registered", component, dep),loc)
            {

            }
    };


    class NetworkException : public CyberForgeException
    {
        public:
            using CyberForgeException::CyberForgeException;
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "Network";
            }
            [[nodiscard]]
            int exitCode() const noexcept override
            {
                return 2;
            }
    };

    class ConnectionException : public NetworkException
    {
        public:
            explicit ConnectionException(std::string_view host,uint16_t port,std::source_location loc = std::source_location::current()): NetworkException(std::format("Cannot connect to {}:{}", host, port),loc), host_(std::string(host)), port_(port)
            {

            }

            [[nodiscard]]
            const std::string& host() const noexcept { return host_; }
            [[nodiscard]]
            uint16_t port() const noexcept { return port_; }

        private:
            std::string host_;
            uint16_t port_;
    };

    class TimeoutException : public NetworkException
    {
        public:
            explicit TimeoutException(std::string_view operation,uint32_t timeoutMs,std::source_location loc = std::source_location::current()): NetworkException(std::format("Timeout after {}ms during '{}'", timeoutMs, operation),loc), operation_(std::string(operation)), timeoutMs_(timeoutMs)
            {

            }

            [[nodiscard]]
            const std::string& operation() const noexcept { return operation_; }
            [[nodiscard]]
            uint32_t timeoutMs() const noexcept { return timeoutMs_; }

        private:
            std::string operation_;
            uint32_t timeoutMs_;
    };

    // --- Безопасность / Сканирование ---

    class ScanException : public CyberForgeException
    {
        public:
            using CyberForgeException::CyberForgeException;
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "Scan";
            }
    };

    class TargetException : public ScanException
    {
        public:
            explicit TargetException(std::string_view target,std::string_view reason,std::source_location loc = std::source_location::current()): ScanException(std::format("Invalid target '{}': {}", target, reason),loc)
            {

            }
    };

    class PermissionException : public CyberForgeException
    {
        public:
            explicit PermissionException(std::string_view operation,std::source_location loc = std::source_location::current()): CyberForgeException(std::format("Permission denied: '{}'", operation),loc)
            {

            }

            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "Permission";
            }

            [[nodiscard]]
            int exitCode() const noexcept override
            {
                return 77;
            }
    };


    class IOException : public CyberForgeException
    {
        public:
            using CyberForgeException::CyberForgeException;
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "IO";
            }
    };

    class FileNotFoundException : public IOException
    {
        public:
            explicit FileNotFoundException(std::string_view path,std::source_location loc = std::source_location::current()): IOException(std::format("File not found: '{}'", path), loc), path_(std::string(path))
            {

            }

            [[nodiscard]]
            const std::string& path() const noexcept
            {
                return path_;
            }

        private:
            std::string path_;
    };


    class NotImplementedException : public CyberForgeException
    {
        public:
            explicit NotImplementedException(std::string_view what = "Not implemented",std::source_location loc = std::source_location::current()): CyberForgeException(std::string(what), loc)
            {

            }

            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "NotImplemented";
            }
    };

    class InvalidArgumentException : public CyberForgeException
    {
        public:
            explicit InvalidArgumentException(std::string_view arg,std::string_view reason,std::source_location loc = std::source_location::current()): CyberForgeException(std::format("Invalid argument '{}': {}", arg, reason),loc)
            {

            }
            [[nodiscard]]
            std::string_view category() const noexcept override
            {
                return "InvalidArgument";
            }
    };

}