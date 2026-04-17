#pragma once

#include <memory>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <stdexcept>
#include <format>
#include <print>
#include <functional>
#include <typeindex>

#include "ServiceProvider.hh"

namespace Tufan::Services
{
    class ServiceManager
    {
        public:
            explicit ServiceManager(Foundation::Application& app);
            ~ServiceManager() = default;
            ServiceManager(const ServiceManager&) = delete;
            ServiceManager& operator=(const ServiceManager&) = delete;

            template<typename Provider, typename... Args> ServiceManager& add(Args&&... args);

            ServiceManager& add(std::unique_ptr<ServiceProvider> provider);

            void registerAll();

            void bootAll();

            void bootstrap();

            void loadDeferredFor(std::string_view componentAlias);

            [[nodiscard]]
            bool has(std::string_view providerName) const noexcept;

            [[nodiscard]]
            std::size_t count() const noexcept;
            [[nodiscard]]
            std::size_t countBooted() const noexcept;
            [[nodiscard]]
            bool allBooted() const noexcept;

            [[nodiscard]]
            std::vector<std::string> providerNames() const;

            [[nodiscard]]
            std::vector<std::string> checkDependencies() const;

            void dump() const;

        private:

            [[nodiscard]]
            std::vector<ServiceProvider*> sortByDependencies() const;

            void topologicalSort(ServiceProvider* provider,std::unordered_set<std::string>& visited,std::unordered_set<std::string>& inStack,std::vector<ServiceProvider*>& result) const;

            Foundation::Application& app_;
            std::vector<std::unique_ptr<ServiceProvider>> providers_;
            std::unordered_map<std::string, ServiceProvider*> byName_;
            std::unordered_map<std::string, ServiceProvider*> eferredMap_;

            bool registered_
            {
                false
            };

            bool booted_
            {
                false
            };
        };


    template<typename Provider, typename... Args> ServiceManager& ServiceManager::add(Args&&... args)
    {
        static_assert(std::is_base_of_v<ServiceProvider, Provider>,"Provider must inherit from ServiceProvider");
        return add(std::make_unique<Provider>(app_, std::forward<Args>(args)...));
    }
}

















