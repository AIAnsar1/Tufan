#include <print>
#include <filesystem>

#include "engine/foundation/Application.hh"


int main(int argc, char* argv[]) {
    std::println("╔══════════════════════════════════════╗");
    std::println("║   Tufan Security Framework           ║");
    std::println("║   v{}                                ║",Tufan::Foundation::Application::frameworkVersion());
    std::println("╚══════════════════════════════════════╝");
    auto app = CyberForge::Foundation::Application::create(std::filesystem::current_path());

    app->onBooting([] {
        std::println("[*] Booting CyberForge...");
    });
    app->onBooted([] {
        std::println("[+] Framework ready.");
    });
    app->bootstrap();
    app->run();
    auto& cfg = app->config();

    if (cfg.has("APP_NAME"))
    {
        std::println("[*] App: {}", cfg.get<std::string>("APP_NAME"));
    }

    if (cfg.has("APP_ENV"))
    {
        std::println("[*] Env: {}", cfg.get<std::string>("APP_ENV"));
    }
    auto& container = app->container();
    std::println("[*] Container bindings: {}", container.size());
    return 0;
}