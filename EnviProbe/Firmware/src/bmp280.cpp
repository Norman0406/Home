#include "bmp280.h"

#include "exceptions.h"

namespace envi_probe {
void BMP280::begin(Configuration &config) {
    m_config = &config;

    m_debugOutput = config.debugOutput();

    if (!m_bmp280.begin(BMP280_ADDRESS_ALT)) {
        throw SensorException("Could not initialize BMP280");
    }

    Serial.println("BMP280 initialized");
}

BMP280::Data BMP280::read() {
    return {m_bmp280.readPressure(), m_bmp280.readTemperature()};
}
}  // namespace envi_probe
