#pragma once


namespace Tufan::Exceptions
{
    class ConfigException : public std::runtime_error
    {
        public:
            explicit ConfigException(const std::string& msg): std::runtime_error("[Config] " + msg) {}
    };
}