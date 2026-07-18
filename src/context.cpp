#include "context.hpp"
#include "logger.hpp"

namespace guchho {

BuildContext::BuildContext(Config config)
    : config_(std::move(config)) {}

const Config& BuildContext::config() const { return config_; }
Config& BuildContext::config() { return config_; }

bool BuildContext::initialize() {
    initialized_ = true;
    log().info("Build context initialized.");
    return true;
}

bool BuildContext::rebuild() {
    log().info("Rebuilding...");
    return true;
}

void BuildContext::dispose() {
    initialized_ = false;
    log().info("Build context disposed.");
}

}
