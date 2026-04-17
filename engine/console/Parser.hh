#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <span>

#include "Command.hh"


namespace Tufan::Console
{

    class ArgumentParser
    {
        public:
            struct ParseResult {
                std::vector<std::string> positional;
                std::unordered_map<std::string, std::string> options;
                bool hasHelp    { false };
                bool hasVersion { false };
            };


            [[nodiscard]]
            static ParseResult parse(std::span<const char* const> argv,const std::vector<Option>&   knownOptions = {});

            [[nodiscard]]
            static CommandContext makeContext(ParseResult result);

        private:
            [[nodiscard]]
            static bool isLongOption(std::string_view arg) noexcept;
            [[nodiscard]]
            static bool isShortOption(std::string_view arg) noexcept;
        };

}