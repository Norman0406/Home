#pragma once

#include <ClosedCube_SHT31D.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class SHT35D {
public:
    SHT35D() = default;

    void begin(Configuration &config);

    struct Data {
        float temperature;
        float humidity;
    };

    Data read();

private:
    bool m_debugOutput{false};
    Configuration *m_config{nullptr};
    ClosedCube_SHT31D m_sht35d;
};
}  // namespace envi_probe
