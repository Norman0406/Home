#pragma once

#include <SparkFunHTU21D.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class HTU21D {
public:
    HTU21D() = default;

    void begin(Configuration &config);

    struct Data {
        float temperature;
        float humidity;
    };

    Data read();

private:
    Configuration *m_config{nullptr};
    ::HTU21D m_htu21d;
};
}  // namespace envi_probe
