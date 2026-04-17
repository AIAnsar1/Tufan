#include <span>
#include <algorithm>

#include "Kernel.hh"
#include "Parser.hh"

namespace Tufan::Console
{

    int ConsoleKernel::handle(int argc, char* argv[])
    {
        if (argc < 2)
        {
            printBanner();
            printHelp();
            return 0;
        }
        std::string_view commandName = argv[1];

        if (commandName == "--help" || commandName == "-h")
        {
            printBanner();
            printHelp();
            return 0;
        }

        if (commandName == "--version" || commandName == "-V")
        {
            std::println("{} v{}", appName_, version_);
            return 0;
        }
        auto it = commands_.find(std::string(commandName));

        if (it == commands_.end())
        {
            std::println(stderr, "\033[91m[!] Unknown command: '{}'\033[0m", commandName);
            std::println(stderr, "    Run '{} --help' to see available commands.", appName_);
            return 1;
        }

        auto& cmd = *it->second;
        const auto rawArgs = std::span<const char* const>(const_cast<const char**>(argv + 2),static_cast<std::size_t>(argc - 2));
        auto parsed = ArgumentParser::parse(rawArgs, cmd.getOptions());

        if (parsed.hasHelp)
        {
            cmd.printHelp();
            return 0;
        }

        if (parsed.hasVersion)
        {
            std::println("{} v{}", cmd.name(), cmd.version());
            return 0;
        }

        // Заполнить дефолты для опций
        for (const auto& opt : cmd.getOptions())
        {
            const std::string key = opt.name.substr(2);

            if (!parsed.options.contains(key) && !opt.defaultValue.empty())
            {
                parsed.options[key] = opt.defaultValue;
            }

            if (opt.shortName && !parsed.options.contains(std::string(1, opt.shortName)))
            {
                if (parsed.options.contains(std::string(1, opt.shortName)))
                {
                    parsed.options[key] = parsed.options[std::string(1, opt.shortName)];
                }
            }
        }
        auto ctx = ArgumentParser::makeContext(std::move(parsed));

        try {
            return cmd.execute(ctx);
        } catch (const std::exception& ex) {
            std::println(stderr, "\033[91m[ERROR] Command '{}' failed: {}\033[0m",commandName, ex.what());
            return 1;
        }
    }

    void ConsoleKernel::printBanner() const
    {
        if (!banner_.empty())
        {
            std::println("{}", banner_);
            return;
        }
        std::println("\033[96m");
        std::println("  ╔═══════════════════════════════════════╗");
        std::println("  ║   ⚡ {} v{}                ║", appName_, version_);
        std::println("  ║   C++23 Security Framework            ║");
        std::println("  ╚═══════════════════════════════════════╝");
        std::println("\033[0m");
    }

    void ConsoleKernel::printHelp() const
    {
        std::println("  \033[93mUsage:\033[0m {} <command> [options]", appName_);
        std::println("");
        std::unordered_map<std::string, std::vector<const Command*>> groups;

        for (const auto& [_, cmd] : commands_)
        {
            groups[std::string(cmd->group())].push_back(cmd.get());
        }

        for (const auto& [group, cmds] : groups)
        {
            std::println("  \033[96m{}:\033[0m", group);

            for (const auto* cmd : cmds)
            {
                std::println("    {:.<20} {}", cmd->name(), cmd->description());
            }
            std::println("");
        }
        std::println("  Run '{} <command> --help' for command details.", appName_);
    }

}