#include <gtest/gtest.h>
#include "cli/cli.hpp"
#include <sstream>
#include <iostream>

TEST(CliTest, PrintUsageContainsHelpText) {
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());
    guchho::print_usage();
    std::cout.rdbuf(old);

    EXPECT_NE(buffer.str().find("Usage:"), std::string::npos);
    EXPECT_NE(buffer.str().find("build"), std::string::npos);
    EXPECT_NE(buffer.str().find("watch"), std::string::npos);
    EXPECT_NE(buffer.str().find("init"), std::string::npos);
    EXPECT_NE(buffer.str().find("version"), std::string::npos);
    EXPECT_NE(buffer.str().find("help"), std::string::npos);
}

TEST(CliTest, PrintVersionOutputsVersion) {
    std::stringstream buffer;
    auto old = std::cout.rdbuf(buffer.rdbuf());
    guchho::print_version();
    std::cout.rdbuf(old);

    EXPECT_NE(buffer.str().find("guchho v"), std::string::npos);
}

TEST(CliTest, RunCliNoArgsReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho")};
    EXPECT_EQ(guchho::run_cli(1, argv), 0);
}

TEST(CliTest, RunCliHelpReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("help")};
    EXPECT_EQ(guchho::run_cli(2, argv), 0);
}

TEST(CliTest, RunCliVersionReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("version")};
    EXPECT_EQ(guchho::run_cli(2, argv), 0);
}

TEST(CliTest, RunCliUnknownReturnsOne) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("unknown")};
    EXPECT_EQ(guchho::run_cli(2, argv), 1);
}

TEST(CliTest, RunCliBuildReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("build")};
    EXPECT_EQ(guchho::run_cli(2, argv), 0);
}

TEST(CliTest, RunCliWatchReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("watch")};
    EXPECT_EQ(guchho::run_cli(2, argv), 0);
}

TEST(CliTest, RunCliInitReturnsZero) {
    char *argv[] = {const_cast<char*>("guchho"), const_cast<char*>("init")};
    EXPECT_EQ(guchho::run_cli(2, argv), 0);
}
