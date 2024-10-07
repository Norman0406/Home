#pragma once

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "configuration.h"

namespace envi_probe {
class Microphone {
public:
    Microphone() = default;

    void begin(Configuration &config);

    struct Data {
        bool isValid;
        float spl;
        float peak;
    };

    Data read();

private:
    void processThread();

    bool m_debugOutput{false};
    Configuration *m_config{nullptr};
    uint8_t m_microphone;
    std::thread m_thread;
    std::mutex m_mutex;

    const int m_resolution{4096};
    const float m_halfResolution = (m_resolution / 2.0f);
    const float m_systemVoltage{3.3f};
    unsigned long m_sumOfSquares{0};
    unsigned long m_numSamples{0};
    long m_peak{0};
};
}  // namespace envi_probe
