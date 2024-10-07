#include "microphone.h"

#include <Arduino.h>

#include <algorithm>

#include "exceptions.h"

namespace envi_probe {
void Microphone::begin(Configuration& config) {
    m_config = &config;

    m_debugOutput = config.debugOutput();

    m_microphone = config.microphone()->pin;

    analogReadResolution(12);

    m_thread = std::thread([this]() { processThread(); });
}

void Microphone::processThread() {
    // process microphone data in batches of 100 ms
    const unsigned long timeFrame = 100;

    try {
        while (true) {
            unsigned long sumOfSquares{0};
            unsigned long numSamples{0};
            long peak{0};

            unsigned long beginTime = millis();

            do {
                long audioValue = analogRead(m_microphone);

                audioValue = (audioValue -
                              (m_halfResolution));  // subtract the dc offset to
                                                    // center the signal at 0

                audioValue *=
                    audioValue;  // square the value to remove negative values

                peak = std::max(peak, audioValue);

                sumOfSquares += audioValue;  // sum the values
                numSamples++;
            } while (millis() - beginTime < timeFrame);

            // lock and update internal value
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_sumOfSquares += sumOfSquares;
                m_numSamples += numSamples;
                m_peak = std::max(m_peak, peak);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(timeFrame));
        }
    } catch (const Exception& e) {
        Serial.println(e.what());
    }
}

Microphone::Data Microphone::read() {
    if (m_numSamples <= 0) {
        return {false, 0, 0};
    }

    // as a reference point, calculate the maximum value for RMS voltage
    // const float maxRawRms = sqrtf(m_halfResolution * m_halfResolution);
    // const float maxRms = maxRawRms * (m_systemVoltage / m_resolution);

    float rms = 0;
    long peak = 0;

    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // divide the sum of the squares to get the mean
        // and then take the square root of the mean
        rms = sqrtf(m_sumOfSquares / (float)m_numSamples);
        peak = sqrtf(m_peak);

        m_sumOfSquares = 0;
        m_numSamples = 0;
        m_peak = 0;
    }

    // convert the RMS value back into a voltage
    // const float rmsVoltage = rms * (m_systemVoltage / m_resolution);

    // convert voltage into decibels
    // const float db = 20 * log10f(rmsVoltage / maxRms);

    // const float splDb = 20 * log10f(rmsVoltage);
    const float splDb =
        20 * log10f(rms / m_halfResolution);  // ? (should be tested)
    const float peakDb = 20 * log10f(peak / m_halfResolution);

    return {true, splDb, peakDb};
}
}  // namespace envi_probe
