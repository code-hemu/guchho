#include "dev/dev_server.hpp"
#include "logger.hpp"

namespace guchho {

DevServer::DevServer(int port, const std::string &host)
    : port_(port), host_(host) {}

bool DevServer::start() {
    log().info("Dev server starting on http://" + host_ + ":" + std::to_string(port_));
    running_ = true;
    return true;
}

void DevServer::stop() {
    running_ = false;
    log().info("Dev server stopped.");
}

bool DevServer::is_running() const { return running_; }

void DevServer::serve_file(const std::string &path, const std::string &content) {
    cached_files_[path] = content;
}

}
