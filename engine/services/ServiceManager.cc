#include <algorithm>
#include <print>
#include <format>
#include <stdexcept>

#include "ServiceManager.hh"
#include "../foundation/Application.hh"

namespace Tufan::Services
{
    ServiceManager::ServiceManager(Foundation::Application& app): app_(app)
    {

    }

    ServiceManager& ServiceManager::add(std::unique_ptr<ServiceProvider> provider)
    {
        if (!provider)
        {
            throw std::invalid_argument("[ServiceManager] Cannot add null provider");
        }
        const std::string name(provider->name());

        if (byName_.contains(name))
        {
            return *this;
        }

        if (provider->isDeferred())
        {
            for (const auto& alias : provider->provides())
            {
                deferredMap_[alias] = provider.get();
            }
        }
        byName_[name] = provider.get();
        providers_.push_back(std::move(provider));
        return *this;
    }


    void ServiceManager::registerAll()
    {
        if (registered_)
        {
            return;
        }
        auto problems = checkDependencies();

        if (!problems.empty())
        {
            for (const auto& p : problems)
            {
                std::println(stderr, "[ServiceManager] WARNING: {}", p);
            }
        }
        auto sorted = sortByDependencies();

        for (auto* provider : sorted)
        {
            if (provider->isRegistered())
            {
                continue;
            }

            if (provider->isDeferred())
            {
                continue;
            }

            try {
                provider->register_();
                provider->markRegistered();
                std::println("[ServiceManager] Registered: \033[96m{}\033[0m",provider->name());
            }
            catch (const std::exception& ex) {
                throw std::runtime_error(std::format("[ServiceManager] Failed to register '{}': {}",provider->name(), ex.what()));
            }
        }
        registered_ = true;
    }

    void ServiceManager::bootAll()
    {
        if (!registered_)
        {
            throw std::logic_error("[ServiceManager] Must call registerAll() before bootAll()");
        }
        if (booted_)
        {
            return;
        }
        auto sorted = sortByDependencies();

        for (auto* provider : sorted)
        {
            if (provider->isBooted())
            {
                continue;
            }

            if (!provider->isRegistered())
            {
                continue;
            }

            try {
                provider->boot();
                provider->markBooted();
                std::println("[ServiceManager] Booted: \033[92m{}\033[0m",provider->name());
            }
            catch (const std::exception& ex)
            {
                throw std::runtime_error(std::format("[ServiceManager] Failed to boot '{}': {}",provider->name(), ex.what()));
            }
        }
        booted_ = true;
    }

    void ServiceManager::bootstrap()
    {
        registerAll();
        bootAll();
    }


    void ServiceManager::loadDeferredFor(std::string_view componentAlias)
    {
        auto it = deferredMap_.find(std::string(componentAlias));

        if (it == deferredMap_.end())
        {
            return;
        }
        auto* provider = it->second;

        if (provider->isBooted())
        {
            return;
        }
        std::println("[ServiceManager] Lazy loading: \033[93m{}\033[0m for '{}'",provider->name(), componentAlias);

        if (!provider->isRegistered())
        {
            provider->register_();
            provider->markRegistered();
        }
        provider->boot();
        provider->markBooted();

        for (const auto& alias : provider->provides())
        {
            deferredMap_.erase(alias);
        }
    }


    bool ServiceManager::has(std::string_view providerName) const noexcept
    {
        return byName_.contains(std::string(providerName));
    }

    std::size_t ServiceManager::count() const noexcept
    {
        return providers_.size();
    }

    std::size_t ServiceManager::countBooted() const noexcept
    {
        return std::ranges::count_if(providers_,[](const auto& p)
        {
            return p->isBooted();
        });
    }

    bool ServiceManager::allBooted() const noexcept
    {
        return std::ranges::all_of(providers_,[](const auto& p)
        {
            return p->isDeferred() || p->isBooted();
        });
    }

    std::vector<std::string> ServiceManager::providerNames() const
    {
        std::vector<std::string> names;
        names.reserve(providers_.size());

        for (const auto& p : providers_)
        {
            names.emplace_back(p->name());
        }
        return names;
    }


    std::vector<std::string> ServiceManager::checkDependencies() const
    {
        std::vector<std::string> problems;

        for (const auto& provider : providers_)
        {
            for (const auto& dep : provider->dependencies())
            {
                if (!byName_.contains(dep))
                {
                    problems.push_back(std::format("'{}' depends on '{}' which is not registered",provider->name(), dep));
                }
            }
        }
        return problems;
    }


    void ServiceManager::dump() const
    {
        std::println("╔══════════════════════════════════════════════════════╗");
        std::println("║           CyberForge Service Providers               ║");
        std::println("╠══════════════════════════════════════════════════════╣");
        std::println("║  Total: {:>3} | Booted: {:>3} | Deferred: {:>3}      ║",providers_.size(),countBooted(),deferredMap_.size());
        std::println("╠══════════════════════════════════════════════════════╣");

        for (const auto& p : providers_)
        {
            const char* status = p->isBooted() ? "\033[92m[booted  ]\033[0m" : p->isRegistered() ? "\033[93m[reg only]\033[0m" : p->isDeferred() ? "\033[96m[deferred]\033[0m" : "\033[91m[pending ]\033[0m";

            std::println("║  {} {}", status, p->name());
            auto deps = p->dependencies();

            if (!deps.empty())
            {
                std::print("║     └─ deps: ");

                for (std::size_t i = 0; i < deps.size(); ++i)
                {
                    std::print("{}{}", deps[i], i + 1 < deps.size() ? ", " : "");
                }
                std::println("");
            }
            auto prov = p->provides();

            if (!prov.empty())
            {
                std::print("║     └─ provides: ");
                for (std::size_t i = 0; i < prov.size(); ++i)
                {
                    std::print("{}{}", prov[i], i + 1 < prov.size() ? ", " : "");
                }
                std::println("");
            }
        }
        std::println("╚══════════════════════════════════════════════════════╝");
    }


    std::vector<ServiceProvider*> ServiceManager::sortByDependencies() const
    {
        std::vector<ServiceProvider*> result;
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> inStack;

        for (const auto& provider : providers_)
        {
            if (!visited.contains(std::string(provider->name())))
            {
                topologicalSort(provider.get(), visited, inStack, result);
            }
        }
        return result;
    }

    void ServiceManager::topologicalSort(ServiceProvider* provider,std::unordered_set<std::string>& visited,std::unordered_set<std::string>& inStack,std::vector<ServiceProvider*>& result) const {
        const std::string name(provider->name());

        if (inStack.contains(name))
        {
            throw std::runtime_error(std::format("[ServiceManager] Circular dependency detected at '{}'", name));
        }

        if (visited.contains(name))
        {
            return;
        }
        inStack.insert(name);

        for (const auto& dep : provider->dependencies())
        {
            auto it = byName_.find(dep);

            if (it != byName_.end())
            {
                topologicalSort(it->second, visited, inStack, result);
            }
        }
        inStack.erase(name);
        visited.insert(name);
        result.push_back(provider);
    }

}