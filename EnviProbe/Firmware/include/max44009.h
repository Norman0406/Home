#pragma once

#include "configuration.h"

#include <max44009/max44009.h>
#include <string>
#include <vector>

namespace envi_probe
{
class Max44009
{
public:
    using Illuminance = float;

    Max44009() = default;

    void begin(Configuration &config);

    Illuminance read(int iterations = 10);

private:
    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    ::Max44009 m_max44009{0x4A};
};
}
