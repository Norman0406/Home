#include "max44009.h"
#include "exceptions.h"

namespace envi_probe
{
void Max44009::begin(Configuration &config)
{
    m_config = &config;

    m_debugOutput = config.debugOutput();
}

Max44009::Illuminance Max44009::read(int iterations)
{
    float illuminance = m_max44009.getLux();

    int error = m_max44009.getError();
    if (error != 0)
    {
        throw SensorException("Could not access Max44009");
    }

    return illuminance;
}
}