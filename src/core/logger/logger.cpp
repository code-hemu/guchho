#include "guchho/logger.hpp"


namespace guchho::logger{
std::string MsgKindToString(MsgKind kind) {
    switch (kind) {
        case MsgKind::kError:   return "ERROR";
        case MsgKind::kWarning: return "WARNING";
        case MsgKind::kInfo:    return "INFO";
        case MsgKind::kNote:    return "NOTE";
        case MsgKind::kDebug:   return "DEBUG";
        case MsgKind::kVerbose: return "VERBOSE";
    }
    return "";
}

}

