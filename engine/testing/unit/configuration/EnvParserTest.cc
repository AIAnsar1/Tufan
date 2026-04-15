#include <gtest/gtest.h>
#include "../../../configuration/EnvParser.hh"

using namespace Tufan::Configuration;

class EnvParserTest : public ::testing::Test {};

TEST_F(EnvParserTest, ParseSimpleKeyValue)
{
    auto env = EnvParser::parseString("APP_NAME=CyberForge\nAPP_ENV=testing");
    EXPECT_EQ(env["APP_NAME"], "CyberForge");
    EXPECT_EQ(env["APP_ENV"],  "testing");
}

TEST_F(EnvParserTest, IgnoreComments)
{
    auto env = EnvParser::parseString("# This is a comment\nAPP_NAME=Test");
    EXPECT_FALSE(env.contains("# This is a comment"));
    EXPECT_EQ(env["APP_NAME"], "Test");
}

TEST_F(EnvParserTest, IgnoreEmptyLines)
{
    auto env = EnvParser::parseString("\n\nAPP_NAME=Test\n\n");
    EXPECT_EQ(env.size(), 1);
    EXPECT_EQ(env["APP_NAME"], "Test");
}

TEST_F(EnvParserTest, ParseDoubleQuotedValue)
{
    auto env = EnvParser::parseString(R"(APP_NAME="My Cyber Tool")");
    EXPECT_EQ(env["APP_NAME"], "My Cyber Tool");
}

TEST_F(EnvParserTest, ParseSingleQuotedValue)
{
    auto env = EnvParser::parseString("APP_NAME='My Cyber Tool'");
    EXPECT_EQ(env["APP_NAME"], "My Cyber Tool");
}

TEST_F(EnvParserTest, ParseBooleanValues)
{
    auto env = EnvParser::parseString("DEBUG=true\nFEATURE=false");
    EXPECT_EQ(env["DEBUG"],   "true");
    EXPECT_EQ(env["FEATURE"], "false");
}

TEST_F(EnvParserTest, ParseInterpolation)
{
    auto env = EnvParser::parseString("DB_HOST=localhost\n""DB_PORT=5432\n""DB_URL=${DB_HOST}:${DB_PORT}");
    EXPECT_EQ(env["DB_URL"], "localhost:5432");
}

TEST_F(EnvParserTest, ParseExportPrefix)
{
    auto env = EnvParser::parseString("export APP_NAME=CyberForge");
    EXPECT_EQ(env["APP_NAME"], "CyberForge");
}

TEST_F(EnvParserTest, InlineCommentIgnored)
{
    auto env = EnvParser::parseString("APP_ENV=production # this is env");
    EXPECT_EQ(env["APP_ENV"], "production");
}