#pragma once

#include <ClosedCube_HDC1080.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class HDC1080 {
public:
    HDC1080() = default;

    void begin(Configuration& config);

    struct Data {
        double temperature;
        double humidity;
    };

    Data read();

private:
    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    ClosedCube_HDC1080 m_hdc1080;
};
}  // namespace envi_probe
