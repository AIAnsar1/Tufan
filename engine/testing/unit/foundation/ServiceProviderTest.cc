#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../../../services/ServiceProvider.hh"
#include "../../../services/ServiceManager.hh"
#include "../../../foundation/Application.hh"

using namespace Tufan::Services;
using namespace Tufan::Foundation;


class ProviderA final : public ServiceProvider
{
    public:
        using ServiceProvider::ServiceProvider;
        inline static int registerCount = 0;
        inline static int bootCount     = 0;

        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "ProviderA";
        }

        void register_() override
        {
            ++registerCount;
        }
        void boot() override
        {
            ++bootCount;
        }
};

class ProviderB final : public ServiceProvider
{
    public:
        using ServiceProvider::ServiceProvider;
        inline static int registerCount = 0;
        inline static int bootCount     = 0;

        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "ProviderB";
        }

        [[nodiscard]]
        std::vector<std::string> dependencies() const noexcept override
        {
            return { "ProviderA" };
        }

        void register_() override
        {
            ++registerCount;
        }
        void boot() override
        {
            ++bootCount;
        }
};

class DeferredProvider final : public ServiceProvider
{
    public:
        using ServiceProvider::ServiceProvider;
        inline static bool registered = false;
        inline static bool booted     = false;

        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "DeferredProvider";
        }

        [[nodiscard]]
        bool isDeferred() const noexcept override
        {
            return true;
        }

        [[nodiscard]]
        std::vector<std::string> provides() const noexcept override
        {
            return { "lazy_service" };
        }

        void register_() override
        {
            registered = true;
        }
        void boot() override
        {
            booted     = true;
        }
};


class ServiceManagerTest : public ::testing::Test
{
    protected:
        void SetUp() override
        {
            ProviderA::registerCount = 0;
            ProviderA::bootCount = 0;
            ProviderB::registerCount = 0;
            ProviderB::bootCount = 0;
            DeferredProvider::registered = false;
            DeferredProvider::booted = false;
            app = Application::create("/tmp/cf_svc_test");
        }
        std::shared_ptr<Application> app;
    };


TEST_F(ServiceManagerTest, AddProvider)
{
    app->services().add<ProviderA>();
    EXPECT_TRUE(app->services().has("ProviderA"));
    EXPECT_EQ(app->services().count(), 1);
}

TEST_F(ServiceManagerTest, RegisterAllCallsRegister)
{
    app->services().add<ProviderA>();
    app->services().registerAll();
    EXPECT_EQ(ProviderA::registerCount, 1);
}

TEST_F(ServiceManagerTest, BootAllCallsBoot)
{
    app->services().add<ProviderA>();
    app->services().registerAll();
    app->services().bootAll();
    EXPECT_EQ(ProviderA::bootCount, 1);
}

TEST_F(ServiceManagerTest, BootWithoutRegisterThrows)
{
    app->services().add<ProviderA>();
    EXPECT_THROW(app->services().bootAll(), std::logic_error);
}

TEST_F(ServiceManagerTest, RegisterIsIdempotent)
{
    app->services().add<ProviderA>();
    app->services().registerAll();
    app->services().registerAll();
    EXPECT_EQ(ProviderA::registerCount, 1);
}

TEST_F(ServiceManagerTest, DependencyOrderRespected)
{
    std::vector<std::string> order;
    app->services().add<ProviderB>();
    app->services().add<ProviderA>();
    app->services().registerAll();
    EXPECT_EQ(ProviderA::registerCount, 1);
    EXPECT_EQ(ProviderB::registerCount, 1);
}

TEST_F(ServiceManagerTest, DuplicateProviderIgnored)
{
    app->services().add<ProviderA>();
    app->services().add<ProviderA>();
    EXPECT_EQ(app->services().count(), 1);
}

TEST_F(ServiceManagerTest, DeferredProviderSkippedOnRegisterAll)
{
    app->services().add<DeferredProvider>();
    app->services().registerAll();
    EXPECT_FALSE(DeferredProvider::registered);
}

TEST_F(ServiceManagerTest, DeferredProviderLoadsOnDemand)
{
    app->services().add<DeferredProvider>();
    app->services().registerAll();
    app->services().bootAll();
    app->services().loadDeferredFor("lazy_service");
    EXPECT_TRUE(DeferredProvider::registered);
    EXPECT_TRUE(DeferredProvider::booted);
}

TEST_F(ServiceManagerTest, CheckDependencies_MissingDep)
{
    app->services().add<ProviderB>();
    auto problems = app->services().checkDependencies();
    EXPECT_EQ(problems.size(), 1);
    EXPECT_THAT(problems[0], ::testing::HasSubstr("ProviderA"));
}

TEST_F(ServiceManagerTest, CheckDependencies_OK)
{
    app->services().add<ProviderA>();
    app->services().add<ProviderB>();
    auto problems = app->services().checkDependencies();
    EXPECT_TRUE(problems.empty());
}

TEST_F(ServiceManagerTest, AllBootedAfterBootstrap)
{
    app->services().add<ProviderA>();
    app->services().add<ProviderB>();
    app->services().bootstrap();
    EXPECT_TRUE(app->services().allBooted());
}

TEST_F(ServiceManagerTest, CircularDependencyDetected)
{
    class ProviderC final : public ServiceProvider
    {
        public:
            using ServiceProvider::ServiceProvider;
            std::string_view name() const noexcept override
            {
                return "ProviderC";
            }
            std::vector<std::string> dependencies() const noexcept override
            {
                return {"ProviderD"};
            }
            void register_() override {}
    };

    class ProviderD final : public ServiceProvider
    {
        public:
            using ServiceProvider::ServiceProvider;
            std::string_view name() const noexcept override
            {
                return "ProviderD";
            }
            std::vector<std::string> dependencies() const noexcept override
            {
                return {"ProviderC"};
            }
            void register_() override {}
    };
    app->services().add<ProviderC>();
    app->services().add<ProviderD>();
    EXPECT_THROW(app->services().registerAll(), std::runtime_error);
}

TEST_F(ServiceManagerTest, ProviderNamesReturnsAll)
{
    app->services().add<ProviderA>();
    app->services().add<ProviderB>();
    auto names = app->services().providerNames();
    EXPECT_THAT(names, ::testing::UnorderedElementsAre("ProviderA", "ProviderB"));
}