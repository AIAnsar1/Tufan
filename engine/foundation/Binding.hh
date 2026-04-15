#pragma once

#include <functional>
#include <memory>
#include <optional>


namespace Tufan::Foundation
{
    struct Binding {
        using FactoryFn = std::function<std::shared_ptr<void>()>;
        FactoryFn factory;
        bool isSingleton = false;
        std::shared_ptr<void> instance = nullptr;

        [[nodiscard]]
        std::shared_ptr<void> resolve()
        {
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
}