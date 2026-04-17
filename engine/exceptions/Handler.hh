#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <optional>

#include "Exceptions.hh"
#include "../logger/ILogger.hh"

namespace Tufan::Exceptions
{
    struct HandlerResult {
        bool handled { false };
        int  exitCode { 0 };
        std::string message;

        [[nodiscard]]
        static HandlerResult ok()
        {
            return { .handled = true, .exitCode = 0 };
        }

        [[nodiscard]]
        static HandlerResult fail(int code, std::string msg)
        {
            return { .handled = true, .exitCode = code, .message = std::move(msg) };
        }

        [[nodiscard]]
        static HandlerResult unhandled()
        {
            return { .handled = false };
        }
    };

    class ExceptionHandler
    {
        public:
            using HandlerFn = std::function<HandlerResult(const std::exception&)>;
            explicit ExceptionHandler(std::shared_ptr<Logger::ILogger> logger = nullptr);

            template<typename ExceptionType> ExceptionHandler& on(std::function<HandlerResult(const ExceptionType&)> handler);
            ExceptionHandler& onAny(std::function<HandlerResult(const std::exception&)> handler);
            ExceptionHandler& onUnhandled(std::function<void(const std::exception&)> handler);
            HandlerResult handle(std::exception_ptr eptr) noexcept;

            template<typename Callable> HandlerResult wrap(Callable&& fn) noexcept;
            void report(const std::exception& ex) const;

        private:
            std::shared_ptr<Logger::ILogger> logger_;
            std::vector<std::pair<std::type_index, HandlerFn>> handlers_;
            HandlerFn anyHandler_;
            std::function<void(const std::exception&)> unhandledHandler_;
            HandlerResult dispatch(const std::exception& ex) noexcept;
    };


    template<typename ExceptionType> ExceptionHandler& ExceptionHandler::on(std::function<HandlerResult(const ExceptionType&)> handler)
    {
        static_assert(std::is_base_of_v<std::exception, ExceptionType>,"ExceptionType must derive from std::exception");

        handlers_.emplace_back(std::type_index(typeid(ExceptionType)),[h = std::move(handler)](const std::exception& ex) -> HandlerResult
        {
                if (const auto* typed = dynamic_cast<const ExceptionType*>(&ex))
                {
                    return h(*typed);
                }
                return HandlerResult::unhandled();
            }
        );
        return *this;
    }

    template<typename Callable> HandlerResult ExceptionHandler::wrap(Callable&& fn) noexcept
    {
        try {
            std::forward<Callable>(fn)();
            return HandlerResult::ok();
        } catch (...) {
            return handle(std::current_exception());
        }
    }

}