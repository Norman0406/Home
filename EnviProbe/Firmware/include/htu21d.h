#pragma once

#include "configuration.h"

#include <SparkFunHTU21D.h>
#include <string>
#include <vector>

namespace envi_probe
{
class HTU21D
{
public:
    HTU21D() = default;

    void begin(Configuration &config);

    struct Data
    {
        float temperature;
        float humidity;
    };

    Data read(int iterations = 10);

private:
    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    ::HTU21D m_htu21d;
};
}
