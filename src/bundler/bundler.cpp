#include "bundler/bundler.hpp"
#include "logger/logger.hpp"

namespace guchho::bundler {

Bundler::Bundler(Config config)
    : config_(std::move(config)) {}

const Config& Bundler::config() const { return config_; }
Config& Bundler::config() { return config_; }

bool Bundler::build() {
    initialized_ = true;
    log().info("Build started");
    return true;
}

bool Bundler::rebuild() {
    log().info("Rebuilding...");
    return true;
}

void Bundler::dispose() {
    initialized_ = false;
    log().info("Build context disposed.");
}

}
