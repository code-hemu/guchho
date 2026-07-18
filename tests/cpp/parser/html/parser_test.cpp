#include <gtest/gtest.h>
#include "html/html_parser.hpp"
#include "html/html_lexer.hpp"

TEST(HtmlParser, ParsesBasicHtml) {
    std::string html = "<html><body></body></html>";
    auto doc = guchho::html::parse_html(html);
    EXPECT_EQ(doc.root.tag, "html");
}

TEST(HtmlParser, HandlesEmptyString) {
    auto doc = guchho::html::parse_html("");
    EXPECT_EQ(doc.root.tag, "html");
}

TEST(HtmlLexer, ProducesTokens) {
    guchho::html::HtmlLexer lexer("<html></html>");
    EXPECT_TRUE(lexer.has_next());
    auto tok = lexer.consume();
    EXPECT_EQ(tok.type, guchho::html::HtmlTokenType::TagOpen);
}

TEST(HtmlLexer, TokenizesAllTokens) {
    std::string html = "<html><body>text</body></html>";
    guchho::html::HtmlLexer lexer(html);
    int count = 0;
    while (lexer.has_next()) {
        lexer.consume();
        ++count;
    }
    EXPECT_GT(count, 0);
}
