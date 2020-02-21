#pragma once

#include "configuration.h"

#include <bsec.h>
#include <string>
#include <vector>

namespace envi_probe
{
class BME680
{
public:
    BME680() = default;

    void begin(Configuration &config);

    struct Data
    {
        float iaq;
        float rawTemperature;
        float pressure;
        float rawHumidity;
        float gasResistance;
        float stabStatus;
        float runInStatus;
        float temperature;
        float humidity;
        float staticIaq;
        float co2Equivalent;
        float breathVocEquivalent;
        float compGasValue;
        float gasPercentage;
        uint8_t iaqAccuracy;
        uint8_t staticIaqAccuracy;
        uint8_t co2Accuracy;
        uint8_t breathVocAccuracy;
        uint8_t compGasAccuracy;
        uint8_t gasPercentageAcccuracy;
    };

    void setTemperatureOffset(float offset);
    Data read();

private:
    void readFromTimer();
    static void readFromTimerStatic(BME680* pThis);
    void loadBsecState(Configuration& config);
    void updateBsecState(Configuration& config);

    bool m_debugOutput{false};
    Configuration* m_config{nullptr};
    Bsec m_bsec;
    uint16_t m_stateUpdateCounter;
    int64_t m_nextCall;
};
}
