#include "max44009.h"

#include "exceptions.h"

namespace envi_probe {
void Max44009::begin(Configuration &config) {
    log_i("Initializing Max44009");

    m_config = &config;

    log_i("Max440909 initialized")
}

Max44009::Illuminance Max44009::read() {
    log_d("Reading data");
    float illuminance = m_max44009.getLux();

    int error = m_max44009.getError();
    if (error != 0) {
        log_e("Max44009 returned error %d", error);
        throw SensorException("Could not access Max44009");
    }

    return illuminance;
}
}  // namespace envi_probe
