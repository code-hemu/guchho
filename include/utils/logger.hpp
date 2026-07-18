#pragma once

#include <iostream>
#include <string>

namespace guchho {

enum class LogLevel {
    Debug,
    Info,
    Warn,
    Error
};

class Logger {
public:
    void set_level(LogLevel level) { level_ = level; }

    void debug(const std::string &msg) { log(LogLevel::Debug, msg); }
    void info(const std::string &msg)  { log(LogLevel::Info,  msg); }
    void warn(const std::string &msg)  { log(LogLevel::Warn,  msg); }
    void error(const std::string &msg) { log(LogLevel::Error, msg); }

private:
    LogLevel level_ = LogLevel::Info;

    void log(LogLevel lvl, const std::string &msg) {
        if (lvl < level_) return;
        switch (lvl) {
            case LogLevel::Debug: std::cout << "[debug] "; break;
            case LogLevel::Info:  std::cout << "[info]  "; break;
            case LogLevel::Warn:  std::cout << "[warn]  "; break;
            case LogLevel::Error: std::cout << "[error] "; break;
        }
        std::cout << msg << std::endl;
    }
};

Logger& log();

}
