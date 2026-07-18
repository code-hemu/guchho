#include <gtest/gtest.h>
#include "bundle/bundle_writer.hpp"
#include <filesystem>

TEST(BundleWriterTest, CreateWriter) {
    guchho::BundleWriter writer(std::filesystem::path("dist"));
    EXPECT_NO_THROW((void)writer);
}

TEST(BundleWriterTest, WriteAllHandlesEmptyList) {
    guchho::BundleWriter writer(std::filesystem::path("dist"));
    EXPECT_TRUE(writer.write_all({}));
}
