#pragma once

#include "../services/ServiceProvider.hh"
#include "ServiceRegistry.hh"

namespace Tufan::Foundation
{


    struct IScanModule {
        virtual ~IScanModule() = default;
        [[nodiscard]] virtual std::string name() const  = 0;
        [[nodiscard]] virtual std::string category() const = 0;
        virtual bool scan(std::string_view target)       = 0;
        virtual void configure(int timeout, bool verbose) {}
    };


    class SqliModule final : public IScanModule
    {
    public:
        [[nodiscard]]
        std::string name()     const override
        {
            return "SQL Injection";
        }
        [[nodiscard]]
        std::string category() const override
        {
            return "web/injection";
        }

        bool scan(std::string_view target) override
        {
            return false;
        }

        void configure(int timeout, bool verbose) override
        {
            timeout_ = timeout;
            verbose_ = verbose;
        }

    private:
        int  timeout_
        {
            30
        };
        bool verbose_
        {
            false
        };
    };

    class XssModule final : public IScanModule
    {
    public:
        [[nodiscard]]
        std::string name() const override
        {
            return "Cross-Site Scripting";
        }
        [[nodiscard]]
        std::string category() const override
        {
            return "web/xss";
        }

        bool scan(std::string_view target) override
        {
            return false;
        }
    };

    class LfiModule final : public IScanModule {
    public:
        [[nodiscard]]
        std::string name() const override
        {
            return "Local File Inclusion";
        }
        [[nodiscard]]
        std::string category() const override
        {
            return "web/lfi";
        }

        bool scan(std::string_view target) override
        {
            return false;
        }
    };


    class ScannerServiceProvider final : public ServiceProvider
    {
    public:
        using ServiceProvider::ServiceProvider;

        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "ScannerServiceProvider";
        }

        [[nodiscard]]
        std::vector<std::string> provides() const noexcept override
        {
            return { "sqli", "xss", "lfi" };
        }

        [[nodiscard]]
        std::vector<std::string> dependencies() const noexcept override
        {
            return
            {
                "LogServiceProvider"
            };
        }

        void register_() override
        {
            registry().module<IScanModule, SqliModule>("sqli", {
                    .name        = "SqliModule",
                    .description = "SQL Injection vulnerability scanner",
                    .version     = "1.0.0",
                    .author      = "CyberForge Team",
                    .type        = Container::ComponentType::Module,
                    .tags        = {"web", "injection", "owasp-a03"}
            }).module<IScanModule, XssModule>("xss", {
                    .name        = "XssModule",
                    .description = "Cross-Site Scripting scanner",
                    .version     = "1.0.0",
                    .author      = "CyberForge Team",
                    .type        = Container::ComponentType::Module,
                    .tags        = {"web", "xss", "owasp-a07"}
                }).module<IScanModule, LfiModule>("lfi", {
                    .name        = "LfiModule",
                    .description = "Local File Inclusion scanner",
                    .version     = "1.0.0",
                    .author      = "CyberForge Team",
                    .type        = Container::ComponentType::Module,
                    .tags        = {"web", "lfi", "owasp-a05"}
            });
        }

        void boot() override
        {
            auto timeout = config().get<int>("SCAN_TIMEOUT",  30);
            auto verbose = config().get<bool>("APP_DEBUG",    false);

            for (const auto& alias : provides())
            {
                auto mod = registry().resolve<IScanModule>(alias);
                mod->configure(timeout, verbose);
            }
            logger().info("[Scanner] {} modules ready (timeout={}s)",provides().size(), timeout);
        }
    };

}