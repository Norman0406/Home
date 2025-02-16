#pragma once

#include <max44009/max44009.h>

#include <string>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class Max44009 {
public:
    using Illuminance = float;

    Max44009() = default;

    void begin(Configuration &config);

    Illuminance read();

private:
    Configuration *m_config{nullptr};
    ::Max44009 m_max44009{0x4A};
};
}  // namespace envi_probe
