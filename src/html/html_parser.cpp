#include "html/html_parser.hpp"
#include "html/html_lexer.hpp"
#include "logger/logger.hpp"
#include <stack>

namespace guchho::html {

HtmlDocument parse_html(const std::string &source) {
    HtmlDocument doc;
    doc.doctype = "html";

    HtmlLexer lexer(source);
    std::vector<HtmlNode> stack;
    HtmlNode root;
    root.tag = "html";
    stack.push_back(root);

    while (lexer.has_next()) {
        auto token = lexer.consume();
        (void)token;
    }

    if (!stack.empty()) {
        doc.root = stack[0];
    }

    log().info("Parsed HTML document");
    return doc;
}

}
