#include "cli_helpers/cli_helpers.hpp"

namespace guchho::cli_helpers {

ErrorWithNote make_error_with_note(const std::string &text, const std::string &note) {
    return {text, note};
}

std::string parse_loader(const std::string &text) {
    if (text == "js") return "js";
    if (text == "jsx") return "jsx";
    if (text == "ts") return "ts";
    if (text == "tsx") return "tsx";
    if (text == "css") return "css";
    if (text == "json") return "json";
    if (text == "text") return "text";
    if (text == "base64") return "base64";
    if (text == "binary") return "binary";
    if (text == "dataurl") return "dataurl";
    if (text == "file") return "file";
    return "default";
}

}
