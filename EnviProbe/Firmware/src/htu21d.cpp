#include "htu21d.h"

#include "exceptions.h"

namespace envi_probe {
void HTU21D::begin(Configuration &config) {
    log_i("Initializing HTU21D");

    m_config = &config;

    m_htu21d.begin();

    log_i("HTU21D initialized");
}

HTU21D::Data HTU21D::read() {
    return {m_htu21d.readTemperature(), m_htu21d.readHumidity()};
}
}  // namespace envi_probe
