#pragma once

#include <Adafruit_Si7021.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class SI7021 {
public:
    SI7021() = default;

    void begin(Configuration &config);

    struct Data {
        float temperature;
        float humidity;
    };

    Data read();

private:
    bool m_debugOutput{false};
    Configuration *m_config{nullptr};
    Adafruit_Si7021 m_si7021;
};
}  // namespace envi_probe
