#include <gtest/gtest.h>
#include "../../../configuration/Repository.hh"

using namespace Tufan::Configuration;

class RepositoryTest : public ::testing::Test
{
    protected:
        Repository repo;
};

TEST_F(RepositoryTest, SetAndGetString)
{
    repo.set("app.name", std::string("CyberForge"));
    EXPECT_EQ(repo.get<std::string>("app.name"), "CyberForge");
}

TEST_F(RepositoryTest, SetAndGetInt)
{
    repo.set("scan.timeout", 30);
    EXPECT_EQ(repo.get<int>("scan.timeout"), 30);
}

TEST_F(RepositoryTest, SetAndGetBool)
{
    repo.set("app.debug", true);
    EXPECT_TRUE(repo.get<bool>("app.debug"));
}

TEST_F(RepositoryTest, GetWithDefault)
{
    EXPECT_EQ(repo.get<int>("missing.key", 42), 42);
}

TEST_F(RepositoryTest, GetOptionalReturnsNulloptForMissing)
{
    auto val = repo.getOptional<std::string>("missing.key");
    EXPECT_FALSE(val.has_value());
}

TEST_F(RepositoryTest, GetOptionalReturnsValueWhenPresent)
{
    repo.set("app.name", std::string("Test"));
    auto val = repo.getOptional<std::string>("app.name");
    ASSERT_TRUE(val.has_value());
    EXPECT_EQ(*val, "Test");
}

TEST_F(RepositoryTest, HasReturnsTrueForExistingKey)
{
    repo.set("app.name", std::string("Test"));
    EXPECT_TRUE(repo.has("app.name"));
}

TEST_F(RepositoryTest, HasReturnsFalseForMissingKey)
{
    EXPECT_FALSE(repo.has("missing.key"));
}

TEST_F(RepositoryTest, ThrowsForMissingKey)
{
    EXPECT_THROW(repo.get<std::string>("missing.key"), ConfigException);
}

TEST_F(RepositoryTest, ConvertStringToBool)
{
    repo.set("flag.true",  std::string("true"));
    repo.set("flag.false", std::string("false"));
    repo.set("flag.one",   std::string("1"));
    repo.set("flag.zero",  std::string("0"));
    EXPECT_TRUE(repo.get<bool>("flag.true"));
    EXPECT_FALSE(repo.get<bool>("flag.false"));
    EXPECT_TRUE(repo.get<bool>("flag.one"));
    EXPECT_FALSE(repo.get<bool>("flag.zero"));
}

TEST_F(RepositoryTest, ConvertStringToInt)
{
    repo.set("timeout", std::string("42"));
    EXPECT_EQ(repo.get<int>("timeout"), 42);
}

TEST_F(RepositoryTest, KeysWithPrefix)
{
    repo.set("app.name",    std::string("Test"));
    repo.set("app.version", std::string("1.0"));
    repo.set("scan.timeout",30);
    auto keys = repo.keysWithPrefix("app.");
    EXPECT_THAT(keys, ::testing::UnorderedElementsAre("app.name", "app.version"));
}

TEST_F(RepositoryTest, RemoveKey)
{
    repo.set("app.name", std::string("Test"));
    EXPECT_TRUE(repo.has("app.name"));
    repo.remove("app.name");
    EXPECT_FALSE(repo.has("app.name"));
}