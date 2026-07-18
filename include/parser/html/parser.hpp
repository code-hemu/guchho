#pragma once

#include "parser/html/ast.hpp"
#include <string>

namespace guchho {

HtmlDocument parse_html(const std::string &content);

}
