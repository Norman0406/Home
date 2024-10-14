#include "sht.h"

#include "exceptions.h"

namespace envi_probe {
void SHT::begin(Configuration &config) {
    Serial.println("Initializing SHT sensor");

    m_debugOutput = config.debugOutput();

    if (!m_sht.init()) {
        throw SensorException("Could not initialize SHT sensor");
    }

    m_sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);

    Serial.println("SHT sensor initialized");
}

SHT::Data SHT::read() {
    if (!m_sht.readSample()) {
        throw SensorException("Could not read SHT sample");
    }

    return {m_sht.getTemperature(), m_sht.getHumidity()};
}
}  // namespace envi_probe
