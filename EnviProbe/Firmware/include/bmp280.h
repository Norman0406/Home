#pragma once

#include <Adafruit_BMP280.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class BMP280 {
public:
    BMP280() = default;

    void begin(Configuration& config);

    struct Data {
        float pressure;
        float temperature;
    };

    Data read();

private:
    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    Adafruit_BMP280 m_bmp280;
};
}  // namespace envi_probe
