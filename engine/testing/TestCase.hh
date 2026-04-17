#pragma once

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <filesystem>
#include <string>
#include <memory>
#include <functional>

#include "../foundation/Application.hh"
#include "../foundation/ServiceRegistry.hh"
#include "../configuration/Repository.hh"
#include "../facades/Config.hh"
#include "../facades/Logger.hh"
#include "../facades/Registry.hh"



namespace Tufan::Testing
{

    class TestCase : public ::testing::Test
    {
        protected:
            void SetUp() override final
            {
                app_ = Foundation::Application::create(testBasePath());
                app_->bootstrap();
                Facades::Config::setApp(app_.get());
                setUp();
            }

            void TearDown() override final
            {
                tearDown();
                Facades::Config::setApp(nullptr);
                app_.reset();
            }

            virtual void setUp(){}
            virtual void tearDown() {}

            void config(std::string_view key, std::string value)
            {
                app_->config().set(key, std::move(value));
            }

            void config(std::string_view key, bool value)
            {
                app_->config().set(key, value);
            }

            void config(std::string_view key, int value)
            {
                app_->config().set(key, value);
            }


            [[nodiscard]]
            Foundation::ServiceRegistry& registry()
            {
                return registry_;
            }

            [[nodiscard]]
            Foundation::Application& app()
            {
                return *app_;
            }

            [[nodiscard]]
            static std::filesystem::path testBasePath()
            {
                return std::filesystem::temp_directory_path() / "tufan_tests";
            }

            [[nodiscard]]
            std::filesystem::path createTempFile(std::string_view name,std::string_view content)
            {
                auto path = testBasePath() / name;
                std::filesystem::create_directories(path.parent_path());
                std::ofstream f(path);
                f << content;
                tempFiles_.push_back(path);
                return path;
            }


            template<typename ExceptionType, typename Callable>void assertThrows(Callable&& fn, std::string_view msgContains = "")
            {
                EXPECT_THROW({
                    try { std::forward<Callable>(fn)(); }
                    catch (const ExceptionType& ex) {
                        if (!msgContains.empty())
                        {
                            EXPECT_THAT(ex.what(), ::testing::HasSubstr(std::string(msgContains)));
                        }
                        throw;
                    }
                }, ExceptionType);
            }

            template<typename Callable>void assertNoThrow(Callable&& fn)
            {
                EXPECT_NO_THROW(std::forward<Callable>(fn)());
            }

        private:
            std::shared_ptr<Foundation::Application> app_;
            Container::ModuleRegistry registry_;
            std::vector<std::filesystem::path> tempFiles_;

            void cleanupTempFiles()
            {
                for (const auto& p : tempFiles_)
                {
                    std::filesystem::remove(p);
                }
                tempFiles_.clear();
            }
    };


    class MockLogger : public Logger::ILogger
    {
        public:
            MOCK_METHOD(void, log,(Logger::LogLevel, std::string_view, std::source_location),(override));
            MOCK_METHOD(void, setLevel, (Logger::LogLevel), (noexcept, override));
            MOCK_METHOD(Logger::LogLevel, getLevel, (), (const, noexcept, override));
    };

}