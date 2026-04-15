
#include "Container.hh"

#include <mutex>


namespace Tufan::Foundation
{
    void Container::bindRaw(std::type_index type, Binding::FactoryFn factory, bool isSingleton)
    {
        std::unique_lock lock(mutex_);
        bindings_[type] = Binding {
            .factory = std::move(factory),
            .isSingleton = isSingleton,
            .instance = nullptr,
        };
    }

    void Container::instanceRaw(std::type_index type, std::shared_ptr<void> inst)
    {
        std::unique_lock lock(mutex_);
        bindings_[type] = Binding {
            .factory     = [inst]()
            {
                return inst;
            },
            .isSingleton = true,
            .instance = inst,
        };
    }

    std::shared_ptr<void> Container::makeRaw(std::type_index type)
    {
        std::shared_lock lock(mutex_);
        auto it = bindings_.find(type);

        if (it == bindings_.end())
        {
            throw Tufan::Exceptions::ContainerException(std::format("No binding registered for type: {}", type.name()));
        }

        if (it->second.isSingleton && it->second.instance)
        {
            return it->second.instance;
        }
        std::unique_lock lock(mutex_);
        auto& binding = bindings_.at(type);
        return binding.resolve();
    }

    bool Container::hasRaw(std::type_index type) const noexcept
    {
        std::shared_lock lock(mutex_);
        return bindings_.contains(type);
    }

    std::size_t Container::size() const noexcept
    {
        std::shared_lock lock(mutex_);
        return bindings_.size();
    }

    bool Container::empty() const noexcept
    {
        std::shared_lock lock(mutex_);
        return bindings_.empty();
    }

    void Container::flush() noexcept
    {
        std::unique_lock lock(mutex_);
        bindings_.clear();
    }

}
