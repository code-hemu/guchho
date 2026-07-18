#include <gtest/gtest.h>
#include "resolver/module_resolver.hpp"
#include <filesystem>

TEST(ModuleResolverTest, CreateResolver) {
    guchho::ModuleResolver resolver({std::filesystem::current_path()});
    EXPECT_NO_THROW(resolver.resolve("test", std::filesystem::current_path()));
}

TEST(ModuleResolverTest, AddAlias) {
    guchho::ModuleResolver resolver({std::filesystem::current_path()});
    EXPECT_NO_THROW(resolver.add_alias("foo", std::filesystem::path("bar")));
}
