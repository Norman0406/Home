#include "si7021.h"

#include "exceptions.h"

namespace envi_probe {
void SI7021::begin(Configuration &config) {
    log_i("Initializing SI7021");

    m_config = &config;

    if (!m_si7021.begin()) {
        throw SensorException("Could not initialize SI7021");
    }

    m_si7021.readSerialNumber();

    log_i("SI7021 S/N: %d-%d, Model: %d, Revision: %d\n", m_si7021.sernum_a,
          m_si7021.sernum_b, static_cast<int>(m_si7021.getModel()),
          m_si7021.getRevision());

    log_i("SI7021 initialized");
}

SI7021::Data SI7021::read() {
    return {m_si7021.readTemperature(), m_si7021.readHumidity()};
}
}  // namespace envi_probe
