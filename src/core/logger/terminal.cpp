#include "guchho/logger.hpp"

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #include <windows.h>
    #include <io.h>

namespace guchho::logger {

static HANDLE HandleFromFD(int file_descriptor)
{
    return reinterpret_cast<HANDLE>(_get_osfhandle(file_descriptor));
}

TerminalInfo GetTerminalInfo(int file_descriptor)
{
    TerminalInfo info{};

    HANDLE handle = HandleFromFD(file_descriptor);

    if (handle != INVALID_HANDLE_VALUE) {
        DWORD unused = 0;
        info.is_tty = (GetConsoleMode(handle, &unused) != FALSE);

        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (GetConsoleScreenBufferInfo(handle, &csbi)) {
            info.width = static_cast<int>(csbi.dwSize.X) - 1;
            info.height = static_cast<int>(csbi.dwSize.Y) - 1;
        }
    }

    // Terminal হলে এবং NO_COLOR সেট না থাকলে ANSI color ব্যবহার করবে।
    info.use_color_escapes =
        info.is_tty && !HasEnvironmentVariableValue("NO_COLOR");

    return info;
}

} 

#elif defined(__APPLE__)
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <termios.h>

namespace guchho::logger {
    TerminalInfo GetTerminalInfo(int file_descriptor) {
        TerminalInfo info{};

        struct termios t;
        if (ioctl(file_descriptor, TIOCGETA, &t) == 0) {
            info.is_tty = true;

            info.use_color_escapes =
                info.is_tty && !HasEnvironmentVariableValue("NO_COLOR");

            struct winsize ws;
            if (ioctl(file_descriptor, TIOCGWINSZ, &ws) == 0) {
                info.width  = ws.ws_col;
                info.height = ws.ws_row;
            }
        }

        return info;
    }
}

#elif defined(__linux__)
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <termios.h>

namespace guchho::logger {

TerminalInfo GetTerminalInfo(int file_descriptor) {
    TerminalInfo info{};

    struct termios t;
    if (ioctl(file_descriptor, TCGETS, &t) == 0) {
        info.is_tty = true;
        info.use_color_escapes =
                info.is_tty && !HasEnvironmentVariableValue("NO_COLOR");

        struct winsize ws;
        if (ioctl(file_descriptor, TIOCGWINSZ, &ws) == 0) {
            info.width  = ws.ws_col;
            info.height = ws.ws_row;
        }
    }

    return info;
}

}

#else
    #include <unistd.h>

namespace guchho::logger {

TerminalInfo GetTerminalInfo(int /*fd*/) {
    return TerminalInfo{};
}

}
#endif

#ifdef _WIN32
namespace guchho::logger {

namespace win32 {

constexpr uint8_t kForegroundBlue = 1 << 0;
constexpr uint8_t kForegroundGreen = 1 << 1;
constexpr uint8_t kForegroundRed = 1 << 2;
constexpr uint8_t kForegroundIntensity = 1 << 3;
constexpr uint8_t kBackgroundBlue = 1 << 4;
constexpr uint8_t kBackgroundGreen = 1 << 5;
constexpr uint8_t kBackgroundRed = 1 << 6;
constexpr uint8_t kBackgroundIntensity = 1 << 7;

struct EscapeEntry {
    std::string_view escape;
    uint16_t attributes;
};

static const EscapeEntry kEscapeMap[] = {
    {"\033[0m",    kForegroundRed | kForegroundGreen | kForegroundBlue},
    {"\033[37m",   kForegroundRed | kForegroundGreen | kForegroundBlue},
    {"\033[1m",    kForegroundRed | kForegroundGreen | kForegroundBlue | kForegroundIntensity},
    {"\033[4m",    kForegroundRed | kForegroundGreen | kForegroundBlue},

    {"\033[31m",   kForegroundRed},
    {"\033[32m",   kForegroundGreen},
    {"\033[34m",   kForegroundBlue},

    {"\033[36m",   kForegroundGreen | kForegroundBlue},
    {"\033[35m",   kForegroundRed | kForegroundBlue},
    {"\033[33m",   kForegroundRed | kForegroundGreen},

    {"\033[41;31m",  kForegroundRed | kBackgroundRed},
    {"\033[41;97m",  kForegroundRed | kForegroundGreen | kForegroundBlue | kBackgroundRed},
    {"\033[42;32m",  kForegroundGreen | kBackgroundGreen},
    {"\033[42;97m",  kForegroundRed | kForegroundGreen | kForegroundBlue | kBackgroundGreen},
    {"\033[44;34m",  kForegroundBlue | kBackgroundBlue},
    {"\033[44;97m",  kForegroundRed | kForegroundGreen | kForegroundBlue | kBackgroundBlue},

    {"\033[46;36m",  kForegroundGreen | kForegroundBlue | kBackgroundGreen | kBackgroundBlue},
    {"\033[46;30m",  kBackgroundGreen | kBackgroundBlue},
    {"\033[45;35m",  kForegroundRed | kForegroundBlue | kBackgroundRed | kBackgroundBlue},
    {"\033[45;30m",  kBackgroundRed | kBackgroundBlue},
    {"\033[43;33m",  kForegroundRed | kForegroundGreen | kBackgroundRed | kBackgroundGreen},
    {"\033[43;30m",  kBackgroundRed | kBackgroundGreen},
};

uint16_t AttributeForEscape(std::string_view escape) {
    for (const auto& entry : kEscapeMap) {
        if (entry.escape == escape) {
            return entry.attributes;
        }
    }
    return 0;
}

}

void WriteStringWithColor(int file_descriptor, const std::string& text) {
    HANDLE handle = HandleFromFD(file_descriptor);
    std::string_view remaining = text;

    while (true) {
        size_t pos = remaining.find('\033');
        if (pos == std::string_view::npos) {
            break;
        }

        std::string_view window = remaining.substr(pos);
        if (window.size() > 8) {
            window = window.substr(0, 8);
        }
        size_t m = window.find('m');
        if (m == std::string_view::npos) {
            remaining = remaining.substr(pos + 1);
            continue;
        }
        m += pos + 1;

        std::string_view escape = remaining.substr(pos, m - pos);
        uint16_t attributes = win32::AttributeForEscape(escape);
        if (attributes == 0 && escape != "\033[0m") {
            remaining = remaining.substr(pos + 1);
            continue;
        }

        std::string_view before = remaining.substr(0, pos);
        if (!before.empty()) {
            DWORD written = 0;
            WriteFile(handle, before.data(), static_cast<DWORD>(before.size()), &written, nullptr);
        }

        SetConsoleTextAttribute(handle, attributes);
        remaining = remaining.substr(m);
    }

    if (!remaining.empty()) {
        DWORD written = 0;
        WriteFile(handle, remaining.data(), static_cast<DWORD>(remaining.size()), &written, nullptr);
    }
}

}
#else
namespace guchho::logger {

void WriteStringWithColor(int file_descriptor, const std::string& text) {
    if (!text.empty()) {
        write(file_descriptor, text.data(), text.size());
    }
}

}

#endif

namespace guchho::logger {

    // Environment variable Value আছে কিনা পরীক্ষা করে।
    bool HasEnvironmentVariableValue(std::string_view name)
    {
    #ifdef _WIN32
        std::string key(name);

        char* value = nullptr;
        size_t size = 0;

        if (_dupenv_s(&value, &size, key.c_str()) != 0 || value == nullptr) {
            return false;
        }

        bool result = value[0] != '\0';
        free(value);

        return result;

    #else
        std::string key(name);
        const char* value = std::getenv(key.c_str());

        return value != nullptr && value[0] != '\0';
    #endif
    }


    // Windows-এ Command Prompt চলছে কিনা তা নির্ধারণ করে.
    //
    // প্রথমবার ফলাফল বের করে সংরক্ষণ করা হয়,
    // তাই পরবর্তী call-এ আবার পরীক্ষা করা লাগে না.
    //
    // WT_SESSION environment variable থাকলে
    // Windows Terminal চলছে বলে ধরা হয়।
    bool IsProbablyWindowsCommandPrompt()
    {
    #ifdef _WIN32
        static const bool result = !HasEnvironmentVariableValue("WT_SESSION");
        return result;
    #else
        return false;
    #endif
    }

}

