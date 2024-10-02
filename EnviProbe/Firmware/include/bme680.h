#pragma once

#include <bsec.h>

#include <string>
#include <vector>

#include "configuration.h"
#include "data.h"

namespace envi_probe {
class BME680 {
public:
    BME680() = default;

    void begin(Configuration &config, envi_probe::Data &data);

    struct Data {
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
    void loadBsecState(envi_probe::Data &config);
    void updateBsecState(envi_probe::Data &config);

    bool m_debugOutput{false};
    envi_probe::Data *m_data{nullptr};
    Bsec m_bsec;
    uint16_t m_stateUpdateCounter;
    int64_t m_nextCall;
};
}  // namespace envi_probe
