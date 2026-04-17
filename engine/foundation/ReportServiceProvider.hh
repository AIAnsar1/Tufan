#pragma once

#include "../services/ServiceProvider.hh"

namespace Tufan::Foundation {


    struct IReporter {
        virtual ~IReporter() = default;
        [[nodiscard]] virtual std::string format()   const = 0;
        virtual void write(std::string_view content) = 0;
        virtual void flush() = 0;
    };


    class JsonReporter final : public IReporter
    {
        public:
            [[nodiscard]]
            std::string format() const override
            {
                return "json";
            }

            void write(std::string_view content) override
            {
                buffer_ += content;
            }

            void flush() override
            {
                buffer_.clear();
            }

        private:
            std::string buffer_;
    };

    class HtmlReporter final : public IReporter
    {
        public:
            [[nodiscard]] std::string format() const override
            {
                return "html";
            }
            void write(std::string_view) override {}
            void flush() override {}
    };

    class CsvReporter final : public IReporter
    {
        public:
            [[nodiscard]]
            std::string format() const override
            {
                return "csv";
            }
            void write(std::string_view) override {}
            void flush() override {}
    };


    class ReportServiceProvider final : public ServiceProvider
    {
        public:
            using ServiceProvider::ServiceProvider;

            [[nodiscard]]
            std::string_view name() const noexcept override
            {
                return "ReportServiceProvider";
            }

            [[nodiscard]]
            std::vector<std::string> provides() const noexcept override
            {
                return { "reporter.json", "reporter.html", "reporter.csv" };
            }

            void register_() override
            {
                registry()
                    .service<IReporter, JsonReporter>("reporter.json", {
                        .name        = "JsonReporter",
                        .description = "JSON report generator",
                        .version     = "1.0.0",
                        .type        = Container::ComponentType::Reporter,
                        .tags        = {"report", "json"}
                    })
                    .service<IReporter, HtmlReporter>("reporter.html", {
                        .name        = "HtmlReporter",
                        .description = "HTML report generator",
                        .version     = "1.0.0",
                        .type        = Container::ComponentType::Reporter,
                        .tags        = {"report", "html"}
                    })
                    .service<IReporter, CsvReporter>("reporter.csv", {
                        .name        = "CsvReporter",
                        .description = "CSV report generator",
                        .version     = "1.0.0",
                        .type        = Container::ComponentType::Reporter,
                        .tags        = {"report", "csv"}
                    });
            }

            void boot() override
            {
                auto format = config().get<std::string>("REPORT_FORMAT", "json");
                logger().info("[Report] Default format: {}", format);
            }
    };

}