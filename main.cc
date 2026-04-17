#include <print>
#include <filesystem>

#include "engine/foundation/Application.hh"
#include "engine/container/ModuleRegistry.hh"
#include "engine/logger/Logger.hh"
#include "engine/console/Kernel.hh"
#include "engine/exceptions/Handler.hh"
#include "engine/facades/Config.hh"
#include "engine/facades/Log.hh"
#include "engine/facades/Registry.hh"



struct IScanModule {
    virtual ~IScanModule()               = default;
    virtual std::string name()   const   = 0;
    virtual bool scan(std::string_view)  = 0;
};

struct SqliScanner : IScanModule {
    std::string name() const override { return "SQL Injection Scanner"; }
    bool scan(std::string_view target) override {
        std::println("  [sqli] Scanning {}...", target);
        return true;
    }
};

struct XssScanner : IScanModule {
    std::string name() const override { return "XSS Scanner"; }
    bool scan(std::string_view target) override {
        std::println("  [xss] Scanning {}...", target);
        return false;
    }
};


class ScanCommand : public Tufan::Console::Command
{
    public:
        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "scan";
        }
        [[nodiscard]]
        std::string_view description() const noexcept override
        {
            return "Scan a target for vulnerabilities";
        }
        [[nodiscard]]
        std::string_view group() const noexcept override
        {
            return "Security";
        }

        void configure() override
        {
            addArgument({ "target",    "Target URL or IP",  true, "" });
            addOption(  { "--module",  'm', "Module alias",  true, "all" });
            addOption(  { "--timeout", 't', "Timeout (sec)", true, "30" });
            addOption(  { "--verbose", 'v', "Verbose output",false, "" });
        }

        int execute(Tufan::Console::CommandContext& ctx) override
        {
            auto target = ctx.arg(0, "localhost");
            auto module = ctx.option("module", std::string("all"));
            std::println("[*] Target: {}", target);
            std::println("[*] Module: {}", module);
            std::println("[*] Scanning...\n");
            return 0;
        }
};

class ListCommand : public Tufan::Console::Command
{
    public:
        [[nodiscard]] std::string_view name() const noexcept override
        {
            return "list";
        }
        [[nodiscard]] std::string_view description() const noexcept override
        {
            return "List registered modules";
        }
        [[nodiscard]] std::string_view group() const noexcept override
        {
            return "Framework";
        }

        void configure() override
        {
            addOption({ "--type", 't', "Filter by type", true, "" });
        }

        int execute(Tufan::Console::CommandContext&) override
        {
            std::println("[*] Registered components:");
            return 0;
        }
    };


int main(int argc, char* argv[]) {


    Tufan::Exceptions::ExceptionHandler handler;
    handler.on<Tufan::Exceptions::NetworkException>([](const Tufan::Exceptions::NetworkException& ex)
    {
        std::println(stderr, "[!] Network error: {}", ex.message());
        return Tufan::Exceptions::HandlerResult::fail(2, ex.message());
    }).on<Tufan::Exceptions::TufanException>([](const Tufan::Exceptions::TufanException& ex)
    {
        std::println(stderr, "[!] [{}] {}", ex.category(), ex.message());
        return Tufan::Exceptions::HandlerResult::fail(1, ex.message());
    }).onAny([](const std::exception& ex) {
        std::println(stderr, "[!] Unexpected: {}", ex.what());
        return Tufan::Exceptions::HandlerResult::fail(1, ex.what());
    });

    return handler.wrap([&]() -> void
    {

        auto app = Tufan::Foundation::Application::create(std::filesystem::current_path());

        Tufan::Logger::LoggerConfig logConfig
        {
            .level = Tufan::Logger::LogLevel::Debug,
            .toConsole = true,
            .colorized = true
        };
        Tufan::Logger::Global::init(logConfig);
        app->bootstrap();
        Tufan::Facades::Config::setApp(app.get());
        Tufan::Foundation::ServiceRegistry registry;
        registry.module<IScanModule, SqliScanner>("sqli", {
            .name        = "SqliScanner",
                .description = "SQL Injection vulnerability scanner",
                .version     = "1.0.0",
                .author      = "CyberForge Team",
                .tags        = {"web", "injection", "owasp-a03"},
                .dependencies= {}
        }).module<IScanModule, XssScanner>("xss", {
                .name        = "XssScanner",
                .description = "Cross-Site Scripting scanner",
                .version     = "1.0.0",
                .author      = "CyberForge Team",
                .tags        = {"web", "xss", "owasp-a07"}
        });

        // Проверка зависимостей
        auto missing = registry.checkDependencies();
        if (!missing.empty())
        {
            for (const auto& m : missing)
            {
                std::println(stderr, "[WARN] Dependency: {}", m);
            }
        }
        registry.dump();
        Tufan::Console::ConsoleKernel console;
        console.setAppName("tufan").setVersion(std::string(app->version())).register_<ScanCommand>().register_<ListCommand>();
        std::exit(console.handle(argc, argv));

    }).exitCode;
}