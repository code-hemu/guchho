#pragma once

#include <string>

namespace guchho::cli_helpers {

struct ErrorWithNote {
    std::string text;
    std::string note;
};

ErrorWithNote make_error_with_note(const std::string &text, const std::string &note);
std::string parse_loader(const std::string &text);

}
