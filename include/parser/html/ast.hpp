#pragma once

#include <memory>
#include <string>
#include <vector>

namespace guchho {

struct Attribute {
    std::string name;
    std::string value;
};

struct HtmlNode {
    std::string tag_name;
    std::vector<Attribute> attributes;
    std::vector<std::unique_ptr<HtmlNode>> children;
};

struct HtmlDocument {
    std::unique_ptr<HtmlNode> root;
    bool has_doctype = false;
};

}
