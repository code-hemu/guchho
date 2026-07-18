#pragma once

#include "config/config.hpp"

namespace guchho {

class BuildContext {
public:
    explicit BuildContext(Config config);

    const Config& config() const;
    Config& config();

    bool initialize();
    bool rebuild();
    void dispose();

private:
    Config config_;
    bool initialized_ = false;
};

}
