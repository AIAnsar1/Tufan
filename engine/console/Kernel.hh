#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <print>

#include "Command.hh"
#include "Parser.hh"

namespace Tufan::Console
{

    class ConsoleKernel {
        public:
            ConsoleKernel()  = default;
            ~ConsoleKernel() = default;

            template<typename Cmd, typename... Args> ConsoleKernel& register_(Args&&... args)
            {
                static_assert(std::is_base_of_v<Command, Cmd>,"Cmd must inherit from Command");
                auto cmd = std::make_unique<Cmd>(std::forward<Args>(args)...);
                cmd->configure();
                const std::string name(cmd->name());
                commands_[name] = std::move(cmd);
                return *this;
            }

            ConsoleKernel& registerCommand(std::unique_ptr<Command> cmd)
            {
                cmd->configure();
                const std::string name(cmd->name());
                commands_[name] = std::move(cmd);
                return *this;
            }

            int handle(int argc, char* argv[]);

            void printBanner() const;
            void printHelp()   const;

            ConsoleKernel& setBanner(std::string banner)
            {
                banner_ = std::move(banner);
                return *this;
            }

            ConsoleKernel& setAppName(std::string name)
            {
                appName_ = std::move(name);
                return *this;
            }

            ConsoleKernel& setVersion(std::string version)
            {
                version_ = std::move(version);
                return *this;
            }

            [[nodiscard]]
            bool hasCommand(std::string_view name) const noexcept
            {
                return commands_.contains(std::string(name));
            }

            [[nodiscard]]
            std::size_t commandCount() const noexcept
            {
                return commands_.size();
            }

        private:
            std::unordered_map<std::string, std::unique_ptr<Command>> commands_;
            std::string appName_ { "CyberForge" };
            std::string version_ { "0.1.0"      };
            std::string banner_;
        };

}