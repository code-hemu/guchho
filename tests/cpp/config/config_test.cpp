#include <gtest/gtest.h>
#include "config/config.hpp"

TEST(ConfigTest, DefaultConfigHasDefaults) {
    guchho::Config cfg;
    EXPECT_EQ(cfg.out_dir, "dist");
    EXPECT_EQ(cfg.out_file, "bundle.js");
    EXPECT_FALSE(cfg.minify);
    EXPECT_FALSE(cfg.source_maps);
    EXPECT_FALSE(cfg.tree_shaking);
    EXPECT_TRUE(cfg.entry_points.empty());
}

TEST(ConfigTest, DefaultConfigValidatesFalse) {
    guchho::Config cfg;
    EXPECT_FALSE(cfg.validate());
}

TEST(ConfigTest, ConfigWithEntryValidatesTrue) {
    guchho::Config cfg;
    cfg.entry_points.push_back("src/index.js");
    EXPECT_TRUE(cfg.validate());
}

TEST(ConfigTest, ConfigWithMultipleEntriesValidatesTrue) {
    guchho::Config cfg;
    cfg.entry_points.push_back("src/index.js");
    cfg.entry_points.push_back("src/app.js");
    EXPECT_TRUE(cfg.validate());
}
