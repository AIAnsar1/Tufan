#include <gtest/gtest.h>
#include "../../../console/Command.hh"
#include "../../..//console/Kernel.hh"
#include "../../..//console/Parser.hh"

using namespace Tufan::Console;


class EchoCommand : public Command
{
    public:
        int lastExitCode = 0;
        std::string lastTarget;
        int lastTimeout = 0;

        [[nodiscard]]
        std::string_view name() const noexcept override
        {
            return "echo";
        }
        [[nodiscard]]
        std::string_view description() const noexcept override
        {
            return "Echo test";
        }

        void configure() override
        {
            addArgument({ "target",  "Target",          true,  "" });
            addOption(  { "--timeout", 't', "Timeout", true, "30" });
            addOption(  { "--verbose", 'v', "Verbose", false, "" });
        }

        int execute(CommandContext& ctx) override
        {
            lastTarget  = ctx.arg(0, "none");
            lastTimeout = ctx.option<int>("timeout", 30);
            return 0;
        }
};


class ConsoleKernelTest : public ::testing::Test {
protected:
    ConsoleKernel kernel;

    void SetUp() override {
        kernel.setAppName("cyberforge").setVersion("0.1.0");
        kernel.register_<EchoCommand>();
    }
};

TEST_F(ConsoleKernelTest, HasRegisteredCommand) {
    EXPECT_TRUE(kernel.hasCommand("echo"));
}

TEST_F(ConsoleKernelTest, UnknownCommandReturns1) {
    const char* argv[] = { "cf", "unknown" };
    EXPECT_EQ(kernel.handle(2, const_cast<char**>(argv)), 1);
}

TEST_F(ConsoleKernelTest, HelpReturns0) {
    const char* argv[] = { "cf", "--help" };
    EXPECT_EQ(kernel.handle(2, const_cast<char**>(argv)), 0);
}

TEST_F(ConsoleKernelTest, VersionReturns0) {
    const char* argv[] = { "cf", "--version" };
    EXPECT_EQ(kernel.handle(2, const_cast<char**>(argv)), 0);
}


class ParserTest : public ::testing::Test {};

TEST_F(ParserTest, ParsePositionalArg) {
    const char* argv[] = { "localhost" };
    auto result = ArgumentParser::parse(std::span(argv, 1));
    ASSERT_EQ(result.positional.size(), 1);
    EXPECT_EQ(result.positional[0], "localhost");
}

TEST_F(ParserTest, ParseLongOptionEquals) {
    const char* argv[] = { "--timeout=60" };
    auto result = ArgumentParser::parse(std::span(argv, 1));
    EXPECT_EQ(result.options.at("timeout"), "60");
}

TEST_F(ParserTest, ParseFlag) {
    const char* argv[] = { "--verbose" };
    auto result = ArgumentParser::parse(std::span(argv, 1));
    EXPECT_EQ(result.options.at("verbose"), "true");
}

TEST_F(ParserTest, ParseHelpFlag) {
    const char* argv[] = { "--help" };
    auto result = ArgumentParser::parse(std::span(argv, 1));
    EXPECT_TRUE(result.hasHelp);
}

TEST_F(ParserTest, ParseVersionFlag) {
    const char* argv[] = { "--version" };
    auto result = ArgumentParser::parse(std::span(argv, 1));
    EXPECT_TRUE(result.hasVersion);
}