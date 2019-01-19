#ifndef SENSORS_H
#define SENSORS_H

namespace envi_probe
{
class Configuration;

class Sensors
{
public:
    Sensors() = default;

    struct Data
    {
        float temperature;
        float humidity;
        float pressure;
        float illuminance;
        float soundLevel;
    };

    Data getSensorData(const Configuration& config) const;
};
}

#endif // SENSORS_H