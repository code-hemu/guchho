#include <gtest/gtest.h>
#include "cli/cli.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

using guchho::cli::CommandHelpText;
using guchho::cli::HelpText;
using guchho::cli::VersionString;

// ---------------------------------------------------------------------------
// Global flags
// ---------------------------------------------------------------------------

TEST(CliTest, NoArgsShowsHelp)
{
    EXPECT_EQ(guchho::cli::Run({}), guchho::cli::kExitOk);
}

TEST(CliTest, HelpFlag)
{
    EXPECT_EQ(guchho::cli::Run({"--help"}), guchho::cli::kExitOk);
    EXPECT_EQ(guchho::cli::Run({"-h"}), guchho::cli::kExitOk);
}

TEST(CliTest, VersionFlag)
{
    EXPECT_EQ(guchho::cli::Run({"--version"}), guchho::cli::kExitOk);
    EXPECT_EQ(guchho::cli::Run({"-v"}), guchho::cli::kExitOk);
}

TEST(CliTest, VersionString)
{
    EXPECT_EQ(VersionString(), "guchho 0.1.0");
}

// ---------------------------------------------------------------------------
// Help text
// ---------------------------------------------------------------------------

TEST(CliTest, HelpTextListsCommands)
{
    auto text = HelpText();
    EXPECT_NE(text.find("Bundle your project for production"), std::string::npos);
    EXPECT_NE(text.find("Create a guchho.config.json"), std::string::npos);
    EXPECT_NE(text.find("Diagnose your environment"), std::string::npos);
}

TEST(CliTest, CommandHelpKnown)
{
    auto text = CommandHelpText("init");
    EXPECT_NE(text.find("guchho init [options] [dir]"), std::string::npos);
    EXPECT_NE(text.find("--force"), std::string::npos);
}

TEST(CliTest, CommandHelpUnknown)
{
    EXPECT_EQ(CommandHelpText("bogus"), "");
}

TEST(CliTest, CommandHelpFlag)
{
    EXPECT_EQ(guchho::cli::Run({"build", "--help"}), guchho::cli::kExitOk);
}

// ---------------------------------------------------------------------------
// Unknown commands
// ---------------------------------------------------------------------------

TEST(CliTest, UnknownCommand)
{
    EXPECT_EQ(guchho::cli::Run({"bogus"}), guchho::cli::kExitUsage);
}

// ---------------------------------------------------------------------------
// Not-implemented stubs
// ---------------------------------------------------------------------------

TEST(CliTest, BuildNotImplemented)
{
    EXPECT_EQ(guchho::cli::Run({"build"}), guchho::cli::kExitError);
}

TEST(CliTest, DevNotImplemented)
{
    EXPECT_EQ(guchho::cli::Run({"dev"}), guchho::cli::kExitError);
}

TEST(CliTest, TestNotImplemented)
{
    EXPECT_EQ(guchho::cli::Run({"test"}), guchho::cli::kExitError);
}

// ---------------------------------------------------------------------------
// init
// ---------------------------------------------------------------------------

static fs::path MakeTempDir()
{
    auto dir = fs::temp_directory_path() / ("guchho_cli_" + std::to_string(::testing::UnitTest::GetInstance()->random_seed()));
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir;
}

TEST(CliInitTest, CreatesConfigFile)
{
    auto dir = MakeTempDir();

    EXPECT_EQ(guchho::cli::Run({"init", dir.string()}), guchho::cli::kExitOk);

    auto path = dir / "guchho.config.json";
    ASSERT_TRUE(fs::exists(path));

    std::ifstream file(path);
    ASSERT_TRUE(file);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("\"outDir\": \"dist\""), std::string::npos);
    EXPECT_NE(contents.find("\"port\": 3000"), std::string::npos);
}

TEST(CliInitTest, RefusesOverwrite)
{
    auto dir = MakeTempDir();

    EXPECT_EQ(guchho::cli::Run({"init", dir.string()}), guchho::cli::kExitOk);
    EXPECT_EQ(guchho::cli::Run({"init", dir.string()}), guchho::cli::kExitError);
}

TEST(CliInitTest, ForceOverwrites)
{
    auto dir = MakeTempDir();

    EXPECT_EQ(guchho::cli::Run({"init", dir.string()}), guchho::cli::kExitOk);

    auto path = dir / "guchho.config.json";
    {
        std::ofstream file(path);
        file << "{\n  \"root\": \"changed\"\n}\n";
    }

    EXPECT_EQ(guchho::cli::Run({"init", "--force", dir.string()}), guchho::cli::kExitOk);

    std::ifstream file(path);
    ASSERT_TRUE(file);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    EXPECT_NE(contents.find("\"outDir\": \"dist\""), std::string::npos);
}

TEST(CliInitTest, UnknownOption)
{
    auto dir = MakeTempDir();
    EXPECT_EQ(guchho::cli::Run({"init", "--bogus", dir.string()}), guchho::cli::kExitUsage);
}

// ---------------------------------------------------------------------------
// doctor
// ---------------------------------------------------------------------------

TEST(CliDoctorTest, Runs)
{
    EXPECT_EQ(guchho::cli::Run({"doctor"}), guchho::cli::kExitOk);
}
