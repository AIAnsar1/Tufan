#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <optional>
#include <functional>
#include <memory>
#include <print>
#include <format>

namespace Tufan::Console
{
    struct Argument
    {
        std::string name;
        std::string description;
        bool required { true };
        std::string defaultValue;
    };

    struct Option {
        std::string name;
        char shortName { 0 };
        std::string description;
        bool hasValue  { true };
        std::string defaultValue;
    };

    class CommandContext
    {
        public:
            CommandContext(std::vector<std::string> args,std::unordered_map<std::string, std::string> options): args_(std::move(args)), options_(std::move(options))
            {

            }

            [[nodiscard]]
            std::optional<std::string>arg(std::size_t index) const noexcept
            {
                if (index < args_.size())
                {
                    return args_[index];
                }
                return std::nullopt;
            }

            [[nodiscard]]
            std::string arg(std::size_t index, std::string_view defaultVal) const noexcept
            {
                return arg(index).value_or(std::string(defaultVal));
            }

            [[nodiscard]]
            bool hasOption(std::string_view name) const noexcept
            {
                return options_.contains(std::string(name));
            }

            [[nodiscard]]
            std::optional<std::string>option(std::string_view name) const noexcept
            {
                auto it = options_.find(std::string(name));

                if (it != options_.end())
                {
                    return it->second;
                }
                return std::nullopt;
            }

            [[nodiscard]]
            std::string option(std::string_view name, std::string_view defaultVal) const noexcept
            {
                return option(name).value_or(std::string(defaultVal));
            }

            template<typename T> [[nodiscard]] T option(std::string_view name, T defaultVal) const noexcept
            {
                auto val = option(name);
                if (!val)
                {
                    return defaultVal;
                }

                if constexpr (std::is_same_v<T, int>)
                {
                    return std::stoi(*val);
                }

                if constexpr (std::is_same_v<T, bool>)
                {
                    return (*val == "true" || *val == "1");
                }

                if constexpr (std::is_same_v<T, double>)
                {
                    return std::stod(*val);
                }
                return defaultVal;
            }

            [[nodiscard]]
            bool flag(std::string_view name) const noexcept
            {
                return hasOption(name);
            }

            [[nodiscard]]
            const std::vector<std::string>& args() const noexcept
            {
                return args_;
            }

        private:
            std::vector<std::string> args_;
            std::unordered_map<std::string, std::string> options_;
    };


    class Command
    {
        public:
            Command() = default;
            virtual ~Command() = default;

            [[nodiscard]]
            virtual std::string_view name() const noexcept = 0;
            [[nodiscard]]
            virtual std::string_view description() const noexcept = 0;
            [[nodiscard]]
            virtual std::string_view group() const noexcept
            {
                return "General";
            }
            [[nodiscard]]
            virtual std::string_view version() const noexcept
            {
                return "1.0.0";
            }

            virtual void configure()
            {

            }

            virtual int execute(CommandContext& ctx) = 0;


            void addArgument(Argument arg)
            {
                arguments_.push_back(std::move(arg));
            }

            void addOption(Option opt)
            {
                options_.push_back(std::move(opt));
            }


            void printHelp() const
            {
                std::println("");
                std::println("  \033[1m{}\033[0m — {}", name(), description());
                std::println("  Version: {} | Group: {}", version(), group());
                std::println("");

                if (!arguments_.empty())
                {
                    std::println("  \033[93mArguments:\033[0m");

                    for (const auto& arg : arguments_)
                    {
                        std::println("{:.<20} {} {}",arg.name,arg.description,arg.required ? "\033[91m(required)\033[0m" : std::format("[{}]", arg.defaultValue));
                    }
                    std::println("");
                }

                if (!options_.empty())
                {
                    std::println("  \033[93mOptions:\033[0m");

                    for (const auto& opt : options_)
                    {
                        std::string shortStr = opt.shortName ? std::format(", -{}", opt.shortName) : "";
                        std::println("    {:.<25} {}  [default: {}]",std::format("{}{}", opt.name, shortStr),opt.description,opt.defaultValue.empty() ? "false" : opt.defaultValue);
                    }
                    std::println("");
                }
            }

            [[nodiscard]]
            const std::vector<Argument>& getArguments() const noexcept
            {
                return arguments_;
            }

            [[nodiscard]]
            const std::vector<Option>&   getOptions()   const noexcept
            {
                return options_;
            }

        private:
            std::vector<Argument> arguments_;
            std::vector<Option> options_;
        };
}