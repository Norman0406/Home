#include "bmp280.h"

#include "exceptions.h"

namespace envi_probe {
void BMP280::begin(Configuration &config) {
    log_i("Initializing BMP280");

    m_config = &config;

    if (!m_bmp280.begin(BMP280_ADDRESS_ALT)) {
        throw SensorException("Could not initialize BMP280");
    }

    log_i("BMP280 initialized");
}

BMP280::Data BMP280::read() {
    log_d("Reading data");
    return {m_bmp280.readPressure(), m_bmp280.readTemperature()};
}
}  // namespace envi_probe
