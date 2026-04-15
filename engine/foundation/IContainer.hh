#pragma once

#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <typeindex>
#include <stdexcept>


namespace Tufan::Foundation
{
    class IContainer
    {
        public:
            virtual ~IContainer() = default;

            template<typename Abstract, typename Factory> void bind(Factory &&factory);
            template<typename Abstract, typename Factory> void singleton(Factory &&factory);
            template<typename Abstract, typename Factory> void instance(std::shared_ptr<Abstract> inst);
            template<typename Abstract> [[nodiscard]] std::shared_ptr<Abstract> make();
            template<typename Abstract> [[nodiscard]] bool has() const noexcept;

        protected:
            using FactoryFn = std::function<std::shared_ptr<void>()>;
            virtual void   bindRaw(std::type_index type, FactoryFn factory, bool isSingleton) = 0;
            virtual void   instanceRaw(std::type_index type, std::shared_ptr<void> inst) = 0;

            [[nodiscard]]
            virtual std::shared_ptr<void> makeRaw(std::type_index type) = 0;
            [[nodiscard]]
            virtual bool   hasRaw(std::type_index type) const noexcept = 0;
    };
}

//
// // Examples
// template<typename Abstract, typename Factory> void IContainer::bind(Factory&& factory)
// {
//     bindRaw(std::type_index(typeid(Abstract)),[f = std::forward<Factory>(factory)]() -> std::shared_ptr<void>
//     {
//         return f();
//     },false);
// }
//
// template<typename Abstract, typename Factory> void IContainer::singleton(Factory&& factory)
// {
//     bindRaw(std::type_index(typeid(Abstract)),[f = std::forward<Factory>(factory)]() -> std::shared_ptr<void>
//     {
//         return f();
//     },true);
// }
//
// template<typename Abstract> void IContainer::instance(std::shared_ptr<Abstract> inst)
// {
//     instanceRaw(std::type_index(typeid(Abstract)), std::move(inst));
// }
//
// template<typename Abstract> [[nodiscard]] std::shared_ptr<Abstract> IContainer::make()
// {
//     auto raw = makeRaw(std::type_index(typeid(Abstract)));
//
//     if (!raw)
//     {
//         throw ContainerException("Cannot resolve: " + std::string(typeid(Abstract).name()));
//     }
//     return std::static_pointer_cast<Abstract>(raw);
// }
//
// template<typename Abstract> [[nodiscard]] bool IContainer::has() const noexcept
// {
//     return hasRaw(std::type_index(typeid(Abstract)));
// }
























