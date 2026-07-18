#include <gtest/gtest.h>
#include "cli/command.hpp"

TEST(CommandTest, BuildWithNoArgsReturnsOne) {
    std::vector<std::string> args;
    EXPECT_EQ(guchho::cmd_build(args), 1);
}

TEST(CommandTest, BuildWithEntryPointReturnsZero) {
    std::vector<std::string> args = {"src/main.js"};
    EXPECT_EQ(guchho::cmd_build(args), 0);
}

TEST(CommandTest, BuildWithOutDirFlag) {
    std::vector<std::string> args = {"--out-dir", "output", "src/main.js"};
    EXPECT_EQ(guchho::cmd_build(args), 0);
}

TEST(CommandTest, BuildWithMinifyFlag) {
    std::vector<std::string> args = {"--minify", "src/main.js"};
    EXPECT_EQ(guchho::cmd_build(args), 0);
}

TEST(CommandTest, BuildWithFormatFlag) {
    std::vector<std::string> args = {"--format", "cjs", "src/main.js"};
    EXPECT_EQ(guchho::cmd_build(args), 0);
}

TEST(CommandTest, BuildWithMultipleFlags) {
    std::vector<std::string> args = {
        "--out-dir", "dist",
        "--minify",
        "--source-maps",
        "--tree-shaking",
        "--format", "esm",
        "src/main.js", "src/app.js"
    };
    EXPECT_EQ(guchho::cmd_build(args), 0);
}

TEST(CommandTest, WatchReturnsZero) {
    std::vector<std::string> args;
    EXPECT_EQ(guchho::cmd_watch(args), 0);
}

TEST(CommandTest, InitReturnsZero) {
    std::vector<std::string> args;
    EXPECT_EQ(guchho::cmd_init(args), 0);
}
