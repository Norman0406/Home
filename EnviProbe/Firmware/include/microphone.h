#pragma once

#include "configuration.h"

#include <string>
#include <vector>

namespace envi_probe
{
class Microphone
{
public:
    using Decibels = float;

    Microphone() = default;

    void begin(Configuration &config);

    Decibels read() const;

private:
    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    uint8_t m_microphone;
};
}
