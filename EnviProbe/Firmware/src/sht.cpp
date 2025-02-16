#include "sht.h"

#include "exceptions.h"

namespace envi_probe {
void SHT::begin(Configuration &config) {
    log_i("Initializing SHT sensor");

    if (!m_sht.init()) {
        throw SensorException("Could not initialize SHT sensor");
    }

    m_sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH);

    log_i("SHT sensor initialized");
}

SHT::Data SHT::read() {
    log_d("Reading data");
    if (!m_sht.readSample()) {
        throw SensorException("Could not read SHT sample");
    }

    return {m_sht.getTemperature(), m_sht.getHumidity()};
}
}  // namespace envi_probe
