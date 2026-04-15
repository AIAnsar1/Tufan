#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>
#include <optional>


namespace Tufan::Configuration
{
    class EnvParser
    {
        public:
            using EnvMap = std::unordered_map<std::string, std::string>;

            [[nodiscard]]
            static EnvMap parse(const std::filesystem::path& path);
            [[nodiscard]]
            static EnvMap parseString(std::string_view content);

        private:
            static std::optional<std::pair<std::string, std::string>>parseLine(std::string_view line, const EnvMap& existing);
            static std::string stripQuotes(std::string_view value);
            static std::string interpolate(std::string_view value, const EnvMap& vars);
        };

}