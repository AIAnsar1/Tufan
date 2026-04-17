#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <shared_mutex>
#include <optional>
#include <print>
#include <format>
#include <typeindex>
#include <stdexcept>


namespace Tufan::Foundation
{
    enum class ComponentType: uint8_t
    {
        Service,
        Module,
        Plugin,
        Driver,
        Middleware,
        Command,
        Reporter,
    };

    [[nodiscard]]
    constexpr std::string_view componentTypeToString(ComponentType t) noexcept
    {
        switch (t)
        {
            case ComponentType::Service:
                return "Service";
            case ComponentType::Module:
                return "Module";
            case ComponentType::Plugin:
                return "Plugin";
            case ComponentType::Driver:
                return "Driver";
            case ComponentType::Middleware:
                return "Middleware";
            case ComponentType::Command:
                return "Command";
            case ComponentType::Reporter:
                return "Reporter";
            default:
                return "Unknown";
        }
    }

    struct ComponentMeta {
        std::string name;
        std::string alias;
        std::string description;
        std::string version;
        std::string author;
        ComponentType type;
        bool enabled { true };
        std::vector<std::string> tags;
        std::vector<std::string> dependencies;
    };

    struct RegistryEntry
    {
        ComponentMeta                           meta;
        std::function<std::shared_ptr<void>()>  factory;
        std::shared_ptr<void>                   instance;   // singleton кеш
        bool                                    isSingleton { false };
        std::type_index                         typeIndex;

        [[nodiscard]]
        std::shared_ptr<void> resolve()
        {
            if (!meta.enabled)
            {
                throw std::runtime_error(std::format("[Registry] Component '{}' is disabled", meta.name));
            }

            if (isSingleton)
            {
                if (!instance)
                {
                     instance = factory();
                }
                return instance;
            }
            return factory();
        }
    };

    class ServiceRegistry
    {
        public:
            ServiceRegistry()  = default;
            ~ServiceRegistry() = default;

            ServiceRegistry(const ServiceRegistry&)            = delete;
            ServiceRegistry& operator=(const ServiceRegistry&) = delete;

            template<typename Abstract, typename Concrete> ServiceRegistry& service(std::string alias, ComponentMeta meta = {});

            template<typename Abstract, typename Concrete> ServiceRegistry& module(std::string alias, ComponentMeta meta = {});

            template<typename Abstract, typename Concrete> ServiceRegistry& driver(std::string alias, ComponentMeta meta = {});

            template<typename Abstract> ServiceRegistry& bind(std::string alias,ComponentType type,std::function<std::shared_ptr<Abstract>()> factory,ComponentMeta meta = {},bool isSingleton = false);

            template<typename Abstract> ServiceRegistry& instance(std::string alias, std::shared_ptr<Abstract> inst, ComponentMeta meta = {});

            template<typename Abstract> [[nodiscard]] std::shared_ptr<Abstract> resolve(std::string_view alias);

            template<typename Abstract> [[nodiscard]] std::vector<std::shared_ptr<Abstract>> resolveByType(ComponentType type);

            template<typename Abstract> [[nodiscard]] std::vector<std::shared_ptr<Abstract>> resolveByTag(std::string_view tag);

            [[nodiscard]]
            bool has(std::string_view alias) const noexcept;

            [[nodiscard]]
            std::optional<ComponentMeta> getMeta(std::string_view alias) const noexcept;

            [[nodiscard]]
            std::vector<ComponentMeta> listAll() const;

            [[nodiscard]]
            std::vector<ComponentMeta> listByType(ComponentType type) const;

            [[nodiscard]]
            std::vector<ComponentMeta> listByTag(std::string_view tag) const;

            void enable(std::string_view alias);
            void disable(std::string_view alias);

            [[nodiscard]]
            bool isEnabled(std::string_view alias) const noexcept;

            [[nodiscard]]
            std::vector<std::string> checkDependencies() const;

            void clear() noexcept;

            [[nodiscard]]
            std::size_t size()  const noexcept;
            [[nodiscard]]
            bool empty() const noexcept;

            void dump() const;

        private:
            mutable std::shared_mutex mutex_;
            std::unordered_map<std::string, RegistryEntry> entries_;
            void registerEntry(std::string alias, RegistryEntry entry);

            [[nodiscard]]
            RegistryEntry* findEntry(std::string_view alias) noexcept;
            [[nodiscard]]
            const RegistryEntry* findEntry(std::string_view alias) const noexcept;
        };



    template<typename Abstract, typename Concrete> ServiceRegistry& ServiceRegistry::service(std::string alias, ComponentMeta meta)
    {
        static_assert(std::is_base_of_v<Abstract, Concrete>, "Concrete must derive from Abstract");
        meta.type  = ComponentType::Service;
        meta.alias = alias;

        if (meta.name.empty())
        {
            meta.name = alias;
        }
        registerEntry(alias, RegistryEntry {
            .meta = std::move(meta),
            .factory = [] { return std::make_shared<Concrete>(); },
            .instance = nullptr,
            .isSingleton = true,
            .typeIndex = std::type_index(typeid(Abstract))
        });
        return *this;
    }

    template<typename Abstract, typename Concrete> ServiceRegistry& ServiceRegistry::module(std::string alias, ComponentMeta meta)
    {
        static_assert(std::is_base_of_v<Abstract, Concrete>,"Concrete must derive from Abstract");
        meta.type  = ComponentType::Module;
        meta.alias = alias;

        if (meta.name.empty())
        {
             meta.name = alias;
        }
        registerEntry(alias, RegistryEntry {
            .meta = std::move(meta),
            .factory = [] { return std::make_shared<Concrete>(); },
            .instance = nullptr,
            .isSingleton = false,
            .typeIndex = std::type_index(typeid(Abstract))
        });
        return *this;
    }

    template<typename Abstract, typename Concrete>ServiceRegistry& ServiceRegistry::driver(std::string alias, ComponentMeta meta)
    {
        static_assert(std::is_base_of_v<Abstract, Concrete>,"Concrete must derive from Abstract");

        meta.type  = ComponentType::Driver;
        meta.alias = alias;

        if (meta.name.empty())
        {
            meta.name = alias;
        }
        registerEntry(alias, RegistryEntry {
            .meta = std::move(meta),
            .factory = [] { return std::make_shared<Concrete>(); },
            .instance = nullptr,
            .isSingleton = true,
            .typeIndex = std::type_index(typeid(Abstract))
        });
        return *this;
    }

    template<typename Abstract>ServiceRegistry& ServiceRegistry::bind(std::string alias,ComponentType type,std::function<std::shared_ptr<Abstract>()> factory,ComponentMeta meta,bool isSingleton)
    {
        meta.type  = type;
        meta.alias = alias;
        if (meta.name.empty()) meta.name = alias;

        registerEntry(alias, RegistryEntry {
            .meta = std::move(meta),
            .factory = [f = std::move(factory)]() -> std::shared_ptr<void> { return f(); },
            .instance = nullptr,
            .isSingleton = isSingleton,
            .typeIndex = std::type_index(typeid(Abstract))
        });
        return *this;
    }

    template<typename Abstract> ServiceRegistry& ServiceRegistry::instance(std::string alias,std::shared_ptr<Abstract> inst,ComponentMeta meta)
    {
        meta.type  = ComponentType::Service;
        meta.alias = alias;

        if (meta.name.empty())
        {
            meta.name = alias;
        }
        auto sharedInst = std::static_pointer_cast<void>(inst);

        registerEntry(alias, RegistryEntry {
            .meta = std::move(meta),
            .factory = [sharedInst]() { return sharedInst; },
            .instance = sharedInst,
            .isSingleton = true,
            .typeIndex = std::type_index(typeid(Abstract))
        });
        return *this;
    }

    template<typename Abstract> [[nodiscard]] std::shared_ptr<Abstract> ServiceRegistry::resolve(std::string_view alias) {
        std::unique_lock lock(mutex_);
        auto* entry = findEntry(alias);

        if (!entry)
        {
            throw std::runtime_error(std::format("[Registry] Component '{}' not found", alias));
        }
        auto raw = entry->resolve();
        return std::static_pointer_cast<Abstract>(raw);
    }

    template<typename Abstract> [[nodiscard]] std::vector<std::shared_ptr<Abstract>> ServiceRegistry::resolveByType(ComponentType type)
    {
        std::unique_lock lock(mutex_);
        std::vector<std::shared_ptr<Abstract>> result;
        const auto targetType = std::type_index(typeid(Abstract));

        for (auto& [alias, entry] : entries_)
        {
            if (entry.meta.type == type && entry.typeIndex == targetType && entry.meta.enabled)
            {
                result.push_back(std::static_pointer_cast<Abstract>(entry.resolve()));
            }
        }
        return result;
    }

    template<typename Abstract> [[nodiscard]] std::vector<std::shared_ptr<Abstract>> ServiceRegistry::resolveByTag(std::string_view tag)
    {
        std::unique_lock lock(mutex_);

        std::vector<std::shared_ptr<Abstract>> result;
        const auto targetType = std::type_index(typeid(Abstract));
        const std::string tagStr(tag);

        for (auto& [alias, entry] : entries_)
        {
            if (entry.typeIndex != targetType || !entry.meta.enabled)
            {
                 continue;
            }
            const auto& tags = entry.meta.tags;

            if (std::ranges::find(tags, tagStr) != tags.end())
            {
                result.push_back(std::static_pointer_cast<Abstract>(entry.resolve()));
            }
        }
        return result;
    }


}



















