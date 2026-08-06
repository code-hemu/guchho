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