#pragma once

#include "../foundation/ServiceRegistry.hh"
#include "../foundation/Application.hh"

namespace Tufan::Facades {


    class Registry
    {
        public:
            Registry() = delete;

            static void setRegistry(Foundation::ServiceRegistry* registry) noexcept
            {
                registry_ = registry;
            }

            template<typename Abstract>[[nodiscard]] static std::shared_ptr<Abstract> resolve(std::string_view alias)
            {
                return get().resolve<Abstract>(alias);
            }

            template<typename Abstract>[[nodiscard]] static std::vector<std::shared_ptr<Abstract>>resolveByType(Foundation::ComponentType type)
            {
                return get().resolveByType<Abstract>(type);
            }

            template<typename Abstract> [[nodiscard]] static std::vector<std::shared_ptr<Abstract>> resolveByTag(std::string_view tag)
            {
                return get().resolveByTag<Abstract>(tag);
            }

            [[nodiscard]]
            static bool has(std::string_view alias) noexcept
            {
                return registry_ && registry_->has(alias);
            }

            static void enable(std::string_view alias)
            {
                get().enable(alias);
            }
            static void disable(std::string_view alias)
            {
                get().disable(alias);
            }
            static void dump() { get().dump(); }

            [[nodiscard]]
            static std::vector<Foundation::ComponentMeta>listAll()
            {
                return get().listAll();
            }

            [[nodiscard]]
            static std::vector<Foundation::ComponentMeta> listByType(Foundation::ComponentType t)
            {
                return get().listByType(t);
            }

        private:
            inline static Foundation::ServiceRegistry* registry_ = nullptr;

            [[nodiscard]] static Foundation::ServiceRegistry& get()
            {
                if (!registry_)
                {
                    throw std::runtime_error("[Registry Facade] Not initialized");
                }
                return *registry_;
            }
    };

}