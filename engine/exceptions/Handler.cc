#include <print>
#include <format>

#include "Handler.hh"


namespace Tufan::Exceptions
{

    ExceptionHandler::ExceptionHandler(std::shared_ptr<Logger::ILogger> logger): logger_(std::move(logger))
    {
        unhandledHandler_ = [this](const std::exception& ex)
        {
            report(ex);
        };
    }

    ExceptionHandler& ExceptionHandler::onAny(std::function<HandlerResult(const std::exception&)> handler)
    {
        anyHandler_ = std::move(handler);
        return *this;
    }

    ExceptionHandler& ExceptionHandler::onUnhandled(std::function<void(const std::exception&)> handler)
    {
        unhandledHandler_ = std::move(handler);
        return *this;
    }

    HandlerResult ExceptionHandler::handle(std::exception_ptr eptr) noexcept
    {
        if (!eptr)
        {
            return HandlerResult::ok();
        }

        try {
            std::rethrow_exception(eptr);
        }
        catch (const CyberForgeException& ex) {
            auto result = dispatch(ex);

            if (!result.handled && unhandledHandler_)
            {
                unhandledHandler_(ex);
            }
            return result;
        }
        catch (const std::exception& ex) {
            auto result = dispatch(ex);

            if (!result.handled && unhandledHandler_)
            {
                unhandledHandler_(ex);
            }
            return result;
        }
        catch (...) {
            HandlerResult r;
            r.handled  = false;
            r.exitCode = 1;
            r.message  = "Unknown exception (non-std)";
            std::println(stderr, "[CRITICAL] Unknown non-std exception caught");
            return r;
        }
    }

    HandlerResult ExceptionHandler::dispatch(const std::exception& ex) noexcept
    {
        for (auto it = handlers_.rbegin(); it != handlers_.rend(); ++it)
        {
            auto result = it->second(ex);

            if (result.handled)
            {
                return result;
            }
        }

        if (anyHandler_)
        {
            return anyHandler_(ex);
        }
        return HandlerResult::unhandled();
    }

    void ExceptionHandler::report(const std::exception& ex) const
    {
        const auto* cfEx = dynamic_cast<const CyberForgeException*>(&ex);

        if (cfEx)
        {
            if (logger_)
            {
                logger_->log(Logger::LogLevel::Error,std::format("[{}] {}", cfEx->category(), cfEx->fullMessage()));
            }
            else
            {
                std::println(stderr, "[ERROR] [{}] {}", cfEx->category(), cfEx->fullMessage());
            }
        }
        else
        {
            if (logger_)
            {
                logger_->log(Logger::LogLevel::Error, ex.what());
            }
            else
            {
                std::println(stderr, "[ERROR] {}", ex.what());
            }
        }
    }

}