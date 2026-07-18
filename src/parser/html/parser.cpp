#include "parser/html/parser.hpp"
#include "parser/html/tokenizer.hpp"
#include "logger.hpp"
#include <functional>
#include <stack>

namespace guchho {

HtmlDocument parse_html(const std::string &content) {
    HtmlTokenizer tokenizer(content);
    HtmlDocument doc;
    doc.root = std::make_unique<HtmlNode>();
    doc.root->tag_name = "document";

    std::stack<HtmlNode*> node_stack;
    node_stack.push(doc.root.get());
    bool seen_doctype = false;

    while (tokenizer.has_next()) {
        Token tok = tokenizer.consume();

        switch (tok.type) {
            case TokenType::Doctype: {
                seen_doctype = true;
                break;
            }
            case TokenType::StartTag: {
                auto node = std::make_unique<HtmlNode>();
                // Extract tag name from lexeme, skipping '<'
                std::string_view l = tok.lexeme;
                size_t name_start = 1; // skip '<'
                size_t name_end = l.find_first_of(" />\t\n", name_start);
                node->tag_name = std::string(l.substr(name_start, name_end - name_start));
                node_stack.top()->children.push_back(std::move(node));
                node_stack.push(node_stack.top()->children.back().get());
                break;
            }
            case TokenType::EndTag: {
                if (!node_stack.empty()) {
                    node_stack.pop();
                }
                if (node_stack.empty()) {
                    node_stack.push(doc.root.get());
                }
                break;
            }
            case TokenType::SelfClosingTag: {
                auto node = std::make_unique<HtmlNode>();
                std::string_view l = tok.lexeme;
                size_t name_start = 1;
                size_t name_end = l.find_first_of(" />\t\n", name_start);
                node->tag_name = std::string(l.substr(name_start, name_end - name_start));
                node_stack.top()->children.push_back(std::move(node));
                break;
            }
            case TokenType::Text: {
                auto node = std::make_unique<HtmlNode>();
                node->tag_name = "#text";
                node_stack.top()->children.push_back(std::move(node));
                break;
            }
            case TokenType::Comment:
            case TokenType::EOFToken:
                break;
            default:
                break;
        }
    }

    doc.has_doctype = seen_doctype;

    int script_count = 0;
    int link_count = 0;
    std::function<void(const HtmlNode&)> count = [&](const HtmlNode& n) {
        if (n.tag_name == "script") ++script_count;
        if (n.tag_name == "link") ++link_count;
        for (const auto& c : n.children) count(*c);
    };
    count(*doc.root);

    log().info("Parsed HTML (" + std::to_string(script_count) + " scripts, "
               + std::to_string(link_count) + " links)");
    return doc;
}

}
