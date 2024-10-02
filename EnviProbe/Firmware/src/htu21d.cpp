#include "htu21d.h"

#include "exceptions.h"

namespace envi_probe {
void HTU21D::begin(Configuration &config) {
    m_config = &config;

    m_debugOutput = config.debugOutput();

    m_htu21d.begin();

    Serial.println("HTU21D initialized");
}

HTU21D::Data HTU21D::read() {
    return {m_htu21d.readTemperature(), m_htu21d.readHumidity()};
}
}  // namespace envi_probe
