#include <algorithm>
#include "Parser.hh"

namespace Tufan::Console
{

    ArgumentParser::ParseResult ArgumentParser::parse(std::span<const char* const> argv,const std::vector<Option>& knownOptions)
    {
        ParseResult result;

        for (std::size_t i = 0; i < argv.size(); ++i)
        {
            std::string_view arg = argv[i];

            if (arg == "--help" || arg == "-h")
            {
                result.hasHelp = true;
                continue;
            }

            if (arg == "--version" || arg == "-V")
            {
                result.hasVersion = true;
                continue;
            }

            if (arg.starts_with("--") && arg.contains('='))
            {
                auto eq= arg.find('=');
                auto key = std::string(arg.substr(2, eq - 2));
                auto value  = std::string(arg.substr(eq + 1));
                result.options[key] = value;
                continue;
            }

            if (arg.starts_with("--"))
            {
                auto key = std::string(arg.substr(2));
                bool hasVal = false;

                for (const auto& opt : knownOptions)
                {
                    if (opt.name == std::string("--") + key && opt.hasValue)
                    {
                        hasVal = true;
                        break;
                    }
                }

                if (hasVal && i + 1 < argv.size() && !std::string_view(argv[i+1]).starts_with('-'))
                {
                    result.options[key] = argv[++i];
                }
                else
                {
                    result.options[key] = "true";
                }
                continue;
            }

            if (arg.starts_with('-') && arg.size() == 2)
            {
                char shortName = arg[1];
                bool hasVal = false;

                for (const auto& opt : knownOptions)
                {
                    if (opt.shortName == shortName && opt.hasValue)
                    {
                        hasVal = true;
                        break;
                    }
                }
                const std::string key(1, shortName);

                if (hasVal && i + 1 < argv.size() && !std::string_view(argv[i+1]).starts_with('-'))
                {
                    result.options[key] = argv[++i];
                }
                else
                {
                    result.options[key] = "true";
                }
                continue;
            }
            result.positional.emplace_back(arg);
        }
        return result;
    }

    CommandContext ArgumentParser::makeContext(ParseResult result)
    {
        return CommandContext(std::move(result.positional), std::move(result.options));
    }

    bool ArgumentParser::isLongOption(std::string_view arg) noexcept
    {
        return arg.starts_with("--");
    }

    bool ArgumentParser::isShortOption(std::string_view arg) noexcept
    {
        return arg.starts_with('-') && arg.size() >= 2 && !arg.starts_with("--");
    }

}