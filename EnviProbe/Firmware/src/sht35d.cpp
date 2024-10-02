#include "sht35d.h"

#include "exceptions.h"

namespace envi_probe {
void SHT35D::begin(Configuration &config) {
    m_config = &config;

    m_debugOutput = config.debugOutput();

    if (m_sht35d.begin(0x44) != SHT31D_ErrorCode::SHT3XD_NO_ERROR) {
        throw SensorException("Could not initialize SHT35D");
    }

    Serial.printf("SHT35D Serial-Number: %d\n", m_sht35d.readSerialNumber());
}

SHT35D::Data SHT35D::read() {
    SHT31D data = m_sht35d.readTempAndHumidity(SHT3XD_REPEATABILITY_HIGH,
                                               SHT3XD_MODE_POLLING, 50);

    return {data.t, data.rh};
}
}  // namespace envi_probe
