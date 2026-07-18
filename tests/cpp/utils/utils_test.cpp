#include <gtest/gtest.h>
#include "logger/logger.hpp"

TEST(LoggerTest, LoggerAvailable) {
    auto &l = guchho::log();
    EXPECT_NO_THROW(l.info("test"));
    EXPECT_NO_THROW(l.warn("test"));
    EXPECT_NO_THROW(l.error("test"));
}
