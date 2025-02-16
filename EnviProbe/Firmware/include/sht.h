#pragma once

#include <arduino-sht.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class SHT {
public:
    SHT() = default;

    void begin(Configuration &config);

    struct Data {
        float temperature;
        float humidity;
    };

    Data read();

private:
    SHTSensor m_sht;
};
}  // namespace envi_probe
