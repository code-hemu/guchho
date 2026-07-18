#include <gtest/gtest.h>
#include "parser/html/parser.hpp"
#include "parser/html/tokenizer.hpp"

TEST(HtmlParser, ParsesBasicHtml) {
    std::string html = "<html><body></body></html>";
    auto doc = guchho::parse_html(html);
    ASSERT_NE(doc.root, nullptr);
    EXPECT_EQ(doc.root->tag_name, "document");
    EXPECT_FALSE(doc.has_doctype);
}

TEST(HtmlParser, DetectsDoctype) {
    std::string html = "<!DOCTYPE html><html></html>";
    auto doc = guchho::parse_html(html);
    EXPECT_TRUE(doc.has_doctype);
}

TEST(HtmlParser, ParsesNestedElements) {
    std::string html = "<html><head><title>Test</title></head></html>";
    auto doc = guchho::parse_html(html);
    ASSERT_GE(doc.root->children.size(), 1);
    EXPECT_EQ(doc.root->children[0]->tag_name, "html");
}

TEST(HtmlParser, CountsScripts) {
    std::string html = R"(<html>
        <script src="app.js"></script>
        <script src="lib.js"></script>
    </html>)";

    auto doc = guchho::parse_html(html);
    ASSERT_GE(doc.root->children.size(), 1);
    auto *html_node = doc.root->children[0].get();
    int script_count = 0;
    std::function<void(const guchho::HtmlNode&)> count =
        [&](const guchho::HtmlNode& n) {
            if (n.tag_name == "script") ++script_count;
            for (const auto& c : n.children) count(*c);
        };
    count(*html_node);
    EXPECT_EQ(script_count, 2);
}

TEST(HtmlParser, CountsLinks) {
    std::string html = R"(<html>
        <link rel="stylesheet" href="style.css">
        <link rel="icon" href="favicon.ico">
    </html>)";

    auto doc = guchho::parse_html(html);
    int link_count = 0;
    std::function<void(const guchho::HtmlNode&)> count =
        [&](const guchho::HtmlNode& n) {
            if (n.tag_name == "link") ++link_count;
            for (const auto& c : n.children) count(*c);
        };
    count(*doc.root);
    EXPECT_EQ(link_count, 2);
}

TEST(HtmlParser, HandlesEmptyString) {
    auto doc = guchho::parse_html("");
    ASSERT_NE(doc.root, nullptr);
}

TEST(HtmlTokenizer, ProducesTokens) {
    guchho::HtmlTokenizer tokenizer("<html></html>");
    EXPECT_TRUE(tokenizer.has_next());
    auto tok = tokenizer.consume();
    EXPECT_EQ(tok.type, guchho::TokenType::StartTag);
}

TEST(HtmlTokenizer, TokenizesAllTokens) {
    std::string html = "<html><body>text</body></html>";
    guchho::HtmlTokenizer tokenizer(html);
    int count = 0;
    while (tokenizer.has_next()) {
        tokenizer.consume();
        ++count;
    }
    EXPECT_GT(count, 0);
}
