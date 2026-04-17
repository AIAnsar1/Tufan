#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../../foundation/Application.hh"
#include "../../../foundation/ModuleRegistry.hh"
#include "../../TestCase.hh"

using namespace Tufan::Foundation;
using namespace Tufan::Testing;


struct IScanModule {
    virtual ~IScanModule() = default;
    virtual std::string name() const   = 0;
    virtual bool scan(std::string_view) = 0;
};

struct SqliModule : IScanModule {
    std::string name() const override { return "SQLi"; }
    bool scan(std::string_view) override { return true; }
};

struct XssModule : IScanModule {
    std::string name() const override { return "XSS"; }
    bool scan(std::string_view) override { return false; }
};

struct IReporter {
    virtual ~IReporter()                      = default;
    virtual std::string report() const        = 0;
};

struct JsonReporter : IReporter {
    std::string report() const override { return R"({"status":"ok"})"; }
};


class RegistryTest : public ::testing::Test
{
    protected:
        ModuleRegistry registry;
};

    TEST_F(RegistryTest, RegisterAndResolveModule)
    {
        registry.module<IScanModule, SqliModule>("sqli", {
            .description = "SQL Injection scanner",
            .tags        = {"web", "injection"}
        });
        EXPECT_TRUE(registry.has("sqli"));
        auto mod = registry.resolve<IScanModule>("sqli");
        EXPECT_EQ(mod->name(), "SQLi");
    }

    TEST_F(RegistryTest, ModuleCreatesNewInstanceEachTime)
    {
        registry.module<IScanModule, SqliModule>("sqli", {});
        auto m1 = registry.resolve<IScanModule>("sqli");
        auto m2 = registry.resolve<IScanModule>("sqli");
        EXPECT_NE(m1.get(), m2.get());
    }

    TEST_F(RegistryTest, ServiceReturnsSameInstance)
    {
        registry.service<IReporter, JsonReporter>("reporter", {});
        auto r1 = registry.resolve<IReporter>("reporter");
        auto r2 = registry.resolve<IReporter>("reporter");
        EXPECT_EQ(r1.get(), r2.get());
    }

    TEST_F(RegistryTest, ResolveByTag)
    {
        registry.module<IScanModule, SqliModule>("sqli", {
            .tags = {"web", "injection"}
        });
        registry.module<IScanModule, XssModule>("xss", {
            .tags = {"web", "xss"}
        });
        auto webModules = registry.resolveByTag<IScanModule>("web");
        EXPECT_EQ(webModules.size(), 2);
    }

    TEST_F(RegistryTest, ResolveByType)
    {
        registry.module<IScanModule, SqliModule>("sqli",     {});
        registry.module<IScanModule, XssModule> ("xss",      {});
        registry.service<IReporter, JsonReporter>("reporter", {});
        auto modules = registry.resolveByType<IScanModule>(ComponentType::Module);
        EXPECT_EQ(modules.size(), 2);
    }

    TEST_F(RegistryTest, DisableComponent)
    {
        registry.module<IScanModule, SqliModule>("sqli", {});
        registry.disable("sqli");
        EXPECT_FALSE(registry.isEnabled("sqli"));
        EXPECT_THROW(registry.resolve<IScanModule>("sqli"), std::runtime_error);
    }

    TEST_F(RegistryTest, EnableComponent)
    {
        registry.module<IScanModule, SqliModule>("sqli", {});
        registry.disable("sqli");
        registry.enable("sqli");

        EXPECT_TRUE(registry.isEnabled("sqli"));
        EXPECT_NO_THROW(registry.resolve<IScanModule>("sqli"));
    }

    TEST_F(RegistryTest, CheckDependencies_Missing)
    {
        registry.module<IScanModule, SqliModule>("sqli", {
            .dependencies = {"http_driver"}  // не зарегистрирован
        });

        auto missing = registry.checkDependencies();
        EXPECT_EQ(missing.size(), 1);
        EXPECT_THAT(missing[0], ::testing::HasSubstr("http_driver"));
    }

    TEST_F(RegistryTest, CheckDependencies_OK)
    {
        registry.service<IReporter, JsonReporter>("json_reporter", {});
        registry.module<IScanModule, SqliModule>("sqli", {
            .dependencies = {"json_reporter"}
        });

        auto missing = registry.checkDependencies();
        EXPECT_TRUE(missing.empty());
    }

    TEST_F(RegistryTest, ListAllReturnsMeta)
    {
        registry.module<IScanModule, SqliModule>("sqli", {
            .description = "SQLi Scanner",
            .version     = "1.0.0"
        });
        registry.service<IReporter, JsonReporter>("reporter", {});

        auto all = registry.listAll();
        EXPECT_EQ(all.size(), 2);
    }

    TEST_F(RegistryTest, ThrowsForUnknownAlias)
    {
        EXPECT_THROW(registry.resolve<IScanModule>("unknown"),std::runtime_error);

    }