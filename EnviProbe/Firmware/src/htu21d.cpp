#include "htu21d.h"
#include "exceptions.h"

namespace envi_probe
{
void HTU21D::begin(Configuration &config)
{
    m_config = &config;

    m_debugOutput = config.debugOutput();

    // initialize HTU21D
    m_htu21d.begin();
}

HTU21D::Data HTU21D::read(int iterations)
{
    float temperature = 0;
    float humidity = 0;
    for (unsigned i = 0; i < iterations; i++)
    {
        float t = m_htu21d.readTemperature();
        float h = m_htu21d.readHumidity();

        if (temperature == ERROR_I2C_TIMEOUT || humidity == ERROR_I2C_TIMEOUT ||
            temperature == ERROR_BAD_CRC || humidity == ERROR_BAD_CRC)
        {
            throw SensorException("Could not access HTU21D");
        }

        temperature += t;
        humidity += h;
    }

    return {
        temperature / iterations,
        humidity / iterations
    };
}
}