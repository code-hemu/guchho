#include <gtest/gtest.h>
#include "css_parser/css_parser.hpp"

TEST(CssParser, ExtractImports) {
    std::string css = R"(
        @import "reset.css";
        @import "theme.css";
        body { color: red; }
    )";
    auto sheet = guchho::css::parse_css(css);
    EXPECT_EQ(sheet.imports.size(), 2);
    EXPECT_EQ(sheet.imports[0], "reset.css");
    EXPECT_EQ(sheet.imports[1], "theme.css");
}

TEST(CssParser, ExtractUrls) {
    std::string css = R"(
        .bg { background: url("bg.png"); }
        .icon { background: url('icon.svg'); }
    )";
    auto sheet = guchho::css::parse_css(css);
    EXPECT_EQ(sheet.urls.size(), 2);
    EXPECT_EQ(sheet.urls[0], "bg.png");
    EXPECT_EQ(sheet.urls[1], "icon.svg");
}

TEST(CssParser, NoImportsOrUrls) {
    std::string css = "body { color: red; }";
    auto sheet = guchho::css::parse_css(css);
    EXPECT_TRUE(sheet.imports.empty());
    EXPECT_TRUE(sheet.urls.empty());
}

TEST(CssParser, PreservesSourceContent) {
    std::string css = "a { color: blue; }";
    auto sheet = guchho::css::parse_css(css);
    EXPECT_EQ(sheet.source, css);
}

TEST(CssParser, HandlesEmptyString) {
    auto sheet = guchho::css::parse_css("");
    EXPECT_TRUE(sheet.imports.empty());
    EXPECT_TRUE(sheet.urls.empty());
}
