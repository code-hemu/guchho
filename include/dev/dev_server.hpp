#pragma once

#include <string>
#include <unordered_map>

namespace guchho {

class DevServer {
public:
    DevServer(int port = 3000, const std::string &host = "localhost");

    bool start();
    void stop();
    bool is_running() const;
    void serve_file(const std::string &path, const std::string &content);

private:
    int port_;
    std::string host_;
    bool running_ = false;
    std::unordered_map<std::string, std::string> cached_files_;
};

}
