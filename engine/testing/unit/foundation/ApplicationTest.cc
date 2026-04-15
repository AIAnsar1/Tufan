#include <gtest/gtest.h>

#include "../../../foundation/Application.hh"

using namespace Tufan::Foundation;

class ApplicationTest : public ::testing::Test
{
    protected:
        void SetUp() override {
            app = Application::create("/tmp/cyberforge_test");
        }
        std::shared_ptr<Application> app;
};

TEST_F(ApplicationTest, InitialStateIsNotBooted)
{
    EXPECT_FALSE(app->isBooted());
    EXPECT_FALSE(app->isRunning());
}

TEST_F(ApplicationTest, BootstrapSetsBootedState)
{
    app->bootstrap();
    EXPECT_TRUE(app->isBooted());
}

TEST_F(ApplicationTest, DoubleBootstrapIsIdempotent)
{
    app->bootstrap();
    app->bootstrap();
    EXPECT_TRUE(app->isBooted());
}

TEST_F(ApplicationTest, RunSetsRunningState)
{
    app->bootstrap();
    app->run();
    EXPECT_TRUE(app->isRunning());
}

TEST_F(ApplicationTest, ContainerIsAccessible)
{
    EXPECT_NO_THROW(app->container());
}

TEST_F(ApplicationTest, ConfigIsAccessible)
{
    EXPECT_NO_THROW(app->config());
}

TEST_F(ApplicationTest, PathCombinesWithBasePath)
{
    auto p = app->path("config/app.cc");
    EXPECT_EQ(p, std::filesystem::path("/tmp/tufan_test/config/app.cc"));
}

TEST_F(ApplicationTest, BootingCallbackIsCalled)
{
    bool called = false;
    app->onBooting([&called] { called = true; });
    app->bootstrap();
    EXPECT_TRUE(called);
}

TEST_F(ApplicationTest, BootedCallbackIsCalled)
{
    bool called = false;
    app->onBooted([&called] { called = true; });
    app->bootstrap();
    EXPECT_TRUE(called);
}