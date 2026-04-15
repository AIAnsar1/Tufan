#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../../foundation/Container.hh"

using namespace Tufan::Foundation;

// ── Тестовые интерфейсы ───────────────────────────────────────────────────────

struct IService {
    virtual ~IService()              = default;
    virtual std::string name() const = 0;
};

struct ConcreteService : IService {
    std::string name() const override { return "ConcreteService"; }
};

struct AnotherService : IService {
    std::string name() const override { return "AnotherService"; }
};

// ── Тесты ────────────────────────────────────────────────────────────────────

class ContainerTest : public ::testing::Test
{
    protected:
        Container container;
};

TEST_F(ContainerTest, BindCreatesNewInstanceEachTime)
{
    container.bind<IService>([] {
        return std::make_shared<ConcreteService>();
    });
    auto s1 = container.make<IService>();
    auto s2 = container.make<IService>();
    EXPECT_NE(s1.get(), s2.get());
    EXPECT_EQ(s1->name(), "ConcreteService");
}

TEST_F(ContainerTest, SingletonReturnsSameInstance)
{
    container.singleton<IService>([] {
        return std::make_shared<ConcreteService>();
    });
    auto s1 = container.make<IService>();
    auto s2 = container.make<IService>();
    EXPECT_EQ(s1.get(), s2.get());
}

TEST_F(ContainerTest, InstanceRegistersExistingObject)
{
    auto svc = std::make_shared<ConcreteService>();
    container.instance<IService>(svc);
    auto resolved = container.make<IService>();
    EXPECT_EQ(resolved.get(), svc.get());
}

TEST_F(ContainerTest, HasReturnsFalseForUnregistered)
{
    EXPECT_FALSE(container.has<IService>());
}

TEST_F(ContainerTest, HasReturnsTrueAfterBind)
{
    container.bind<IService>([] {
        return std::make_shared<ConcreteService>();
    });
    EXPECT_TRUE(container.has<IService>());
}

TEST_F(ContainerTest, MakeThrowsForUnregisteredType)
{
    EXPECT_THROW(container.make<IService>(), ContainerException);
}

TEST_F(ContainerTest, FlushClearsAllBindings)
{
    container.singleton<IService>([] {
        return std::make_shared<ConcreteService>();
    });
    EXPECT_EQ(container.size(), 1);
    container.flush();
    EXPECT_EQ(container.size(), 0);
    EXPECT_TRUE(container.empty());
}

TEST_F(ContainerTest, OverrideBinding)
{
    container.bind<IService>([] {
        return std::make_shared<ConcreteService>();
    });
    container.bind<IService>([] {
        return std::make_shared<AnotherService>();
    });
    auto svc = container.make<IService>();
    EXPECT_EQ(svc->name(), "AnotherService");
}