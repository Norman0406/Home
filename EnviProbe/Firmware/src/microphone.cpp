#include "microphone.h"
#include "exceptions.h"

#include <Arduino.h>

namespace envi_probe
{
void Microphone::begin(Configuration &config)
{
    m_config = &config;

    m_debugOutput = config.debugOutput();

    m_microphone = config.microphone();

    analogReadResolution(12);
}

Microphone::Decibels Microphone::read() const
{
    const int resolution = 4096;
    const float systemVoltage = 3.3f;

    // as a reference point, calculate the maximum value for RMS voltage
    const float halfResolution = (resolution / 2.0f);
    const float maxRawRms = sqrtf(halfResolution * halfResolution);
    const float maxRms = maxRawRms * (systemVoltage / resolution);

    unsigned long sumOfSquares = 0;

    unsigned long beginTime = millis();
    const unsigned long timeFrame = 50;

    unsigned long numSamples = 0;
    do {
        int audioValue = analogRead(m_microphone);

        audioValue = (audioValue - (resolution / 2));  // subtract the dc offset to center the signal at 0
        audioValue *= audioValue;               // square the value to remove negative values
        sumOfSquares += audioValue;         // sum the values
        numSamples++;
    } while (millis() - beginTime < timeFrame);

    // divide the sum of the squares to get the mean
    // and then take the square root of the mean
    const float rms = sqrtf(sumOfSquares / (float)numSamples);

    // convert the RMS value back into a voltage
    const float rmsVoltage = rms * (systemVoltage / resolution);

    // convert voltage into decibels
    const float db = 20 * log10f(rmsVoltage / maxRms);

    return db;
}
}