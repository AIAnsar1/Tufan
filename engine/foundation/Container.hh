#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <typeindex>
#include <format>

#include "../../kernel/foundation/IContainer.hh"
#include "Binding.hh"

namespace Tufan::Foundation
{
    class Container
    {
        public:
            Container() = default;
            ~Container() = default;
            Container(const Container&) = delete;
            Container* operator=(const Container&) = delete;
            Container(Container&&) = default;
            Container& operator=(Container&&) = default;

            [[nodiscard]]
            std::size_t size() const noexcept;
            [[nodiscard]]
            bool empty() const noexcept;
            void flush() noexcept;


        protected:
            void bindRaw(std::type_index type,Binding::FactoryFn factory,bool isSingleton) override;
            void instanceRaw(std::type_index type,std::shared_ptr<void> inst) override;

            [[nodiscard]]
            std::shared_ptr<void> makeRaw(std::type_index type) override;

            [[nodiscard]]
            bool hasRaw(std::type_index type) const noexcept override;

        private:
            mutable std::shared_mutex mutex_;
            std::unordered_map<std::type_index, Binding> bindings_;
        };
}
















