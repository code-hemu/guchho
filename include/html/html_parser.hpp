#pragma once

#include "html/html_ast.hpp"
#include <string>

namespace guchho::html {

HtmlDocument parse_html(const std::string &source);

}
