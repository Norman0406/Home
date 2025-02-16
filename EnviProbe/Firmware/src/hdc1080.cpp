#include "hdc1080.h"

#include "exceptions.h"

namespace envi_probe {
void HDC1080::begin(Configuration &config) {
    log_i("Initializing HDC1080");

    m_config = &config;

    m_hdc1080.begin(0x40);

    auto serialNumber = m_hdc1080.readSerialNumber();

    log_i("HDC1080 S/N: %d-%d-%d, Device Id: %d\n", serialNumber.serialFirst,
          serialNumber.serialMid, serialNumber.serialLast,
          m_hdc1080.readDeviceId());

    log_i("HDC1080 initialized")
}

HDC1080::Data HDC1080::read() {
    return {m_hdc1080.readTemperature(), m_hdc1080.readHumidity()};
}
}  // namespace envi_probe
