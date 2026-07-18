#pragma once

#include "config/config.hpp"

namespace guchho::bundler {

class Bundler {
public:
    explicit Bundler(Config config);

    bool build();
    bool rebuild();
    void dispose();

    const Config& config() const;
    Config& config();

private:
    Config config_;
    bool initialized_ = false;
};

}
