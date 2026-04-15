#pragma once

#include <memory>
#include <vector>

#include "IApplication.hh"


namespace Tufan::Foundation
{
    class IServiceProvider
    {
        public:
            explicit IServiceProvider(IApplication& app) : app_(app) {}
            virtual ~IServiceProvider() = default;
            virtual void register_() = 0;
            virtual void boot() = 0;
            [[nodiscard]]
            virtual std::vector<std::string> provides() const noexcept
            {
                return {};
            }

            [[nodiscard]]
            virtual bool isDeferred() const noexcept
            {
                return false;
            }

        protected:
            IApplication& app_;
        };

}