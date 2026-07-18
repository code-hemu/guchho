#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace guchho::html {

struct HtmlAttribute {
    std::string name;
    std::string value;
};

struct HtmlNode {
    std::string tag;
    std::vector<HtmlAttribute> attributes;
    std::string content;
    std::vector<HtmlNode> children;
};

struct HtmlDocument {
    std::string doctype;
    HtmlNode root;
};

}
