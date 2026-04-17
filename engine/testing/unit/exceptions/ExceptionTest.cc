#include <gtest/gtest.h>
#include "../../../exceptions/Exceptions.hh"
#include "../../../exceptions/Handler.hh"

using namespace Tufan::Exceptions;

class ExceptionTest : public ::testing::Test {};

TEST_F(ExceptionTest, BaseExceptionHasMessage)
{
    CyberForgeException ex("test error");
    EXPECT_EQ(ex.message(), "test error");
    EXPECT_EQ(ex.category(), "CyberForge");
}

TEST_F(ExceptionTest, ConnectionExceptionHasHostAndPort)
{
    ConnectionException ex("192.168.1.1", 8080);
    EXPECT_EQ(ex.host(), "192.168.1.1");
    EXPECT_EQ(ex.port(), 8080);
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("192.168.1.1"));
    EXPECT_THAT(ex.what(), ::testing::HasSubstr("8080"));
}

TEST_F(ExceptionTest, TimeoutExceptionHasDetails)
{
    TimeoutException ex("port_scan", 5000);
    EXPECT_EQ(ex.operation(), "port_scan");
    EXPECT_EQ(ex.timeoutMs(), 5000u);
}

TEST_F(ExceptionTest, FileNotFoundHasPath)
{
    FileNotFoundException ex("/etc/secret");
    EXPECT_EQ(ex.path(), "/etc/secret");
}


class HandlerTest : public ::testing::Test
{
    protected:
        ExceptionHandler handler;
};

TEST_F(HandlerTest, HandlesRegisteredExceptionType)
{
    bool handled = false;

    handler.on<NetworkException>([&](const NetworkException& ex) {
        handled = true;
        return HandlerResult::ok();
    });
    auto result = handler.handle(
        std::make_exception_ptr(ConnectionException("localhost", 80))
    );
    EXPECT_TRUE(handled);
    EXPECT_TRUE(result.handled);
}

TEST_F(HandlerTest, WrapCatchesException)
{
    handler.on<CyberForgeException>([](const CyberForgeException&) {
        return HandlerResult::fail(1, "caught");
    });
    auto result = handler.wrap([]{
        throw CyberForgeException("boom");
    });
    EXPECT_TRUE(result.handled);
    EXPECT_EQ(result.exitCode, 1);
}

TEST_F(HandlerTest, WrapReturnsOkWhenNoException)
{
    auto result = handler.wrap([]{ /* ничего */ });
    EXPECT_TRUE(result.handled);
    EXPECT_EQ(result.exitCode, 0);
}

TEST_F(HandlerTest, OnAnyHandlesCatchAll)
{
    handler.onAny([](const std::exception& ex) {
        return HandlerResult::fail(99, ex.what());
    });
    auto result = handler.handle(
        std::make_exception_ptr(std::runtime_error("generic"))
    );
    EXPECT_TRUE(result.handled);
    EXPECT_EQ(result.exitCode, 99);
}