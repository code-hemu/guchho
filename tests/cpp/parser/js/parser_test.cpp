#include <gtest/gtest.h>
#include "parser/js/parser.hpp"

TEST(JsParser, ExtractDefaultImports) {
    std::string source = R"(import foo from "./foo.js";)";
    auto mod = guchho::parse_js(source);
    EXPECT_EQ(mod.imports.size(), 1);
    EXPECT_EQ(mod.imports[0], "./foo.js");
}

TEST(JsParser, ExtractNamedImports) {
    std::string source = R"(import { readFile, writeFile } from "fs";)";
    auto mod = guchho::parse_js(source);
    EXPECT_EQ(mod.imports.size(), 1);
    EXPECT_EQ(mod.imports[0], "fs");
}

TEST(JsParser, ExtractMultipleImports) {
    std::string source = R"(
        import { createServer } from "http";
        import path from "path";
        import "./polyfill.js";
    )";
    auto mod = guchho::parse_js(source);
    EXPECT_EQ(mod.imports.size(), 3);
    EXPECT_EQ(mod.imports[0], "http");
    EXPECT_EQ(mod.imports[1], "path");
    EXPECT_EQ(mod.imports[2], "./polyfill.js");
}

TEST(JsParser, DetectsExports) {
    std::string source = R"(export const x = 42;)";
    auto mod = guchho::parse_js(source);
    EXPECT_FALSE(mod.exports.empty());
}

TEST(JsParser, NoImportsReturnsEmpty) {
    std::string source = R"(const x = 1; const y = 2;)";
    auto mod = guchho::parse_js(source);
    EXPECT_TRUE(mod.imports.empty());
}

TEST(JsParser, HandlesEmptyString) {
    auto mod = guchho::parse_js("");
    EXPECT_TRUE(mod.imports.empty());
    EXPECT_TRUE(mod.exports.empty());
}
