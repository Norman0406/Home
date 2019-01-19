#include <sensors.h>
#include <exceptions.h>
#include <configuration.h>
#include <max44009.h>

#include <Adafruit_BMP280.h>
#include <SparkFunHTU21D.h>

namespace envi_probe
{
namespace
{
    struct HTU21DData
    {
        float temperature;
        float humidity;
    };

    HTU21DData getHTU21DData(unsigned iterations = 10)
    {
        HTU21D htu21d;

        htu21d.begin();

        float temperature = 0;
        float humidity = 0;
        for (unsigned i = 0; i < iterations; i++)
        {
            float t = htu21d.readTemperature();
            float h = htu21d.readHumidity();

            if (temperature == ERROR_I2C_TIMEOUT || humidity == ERROR_I2C_TIMEOUT ||
                temperature == ERROR_BAD_CRC || humidity == ERROR_BAD_CRC)
            {
                throw SensorException("Could not access HTU21D");
            }

            temperature += t;
            humidity += h;
        }

        return {temperature / iterations, humidity / iterations};
    }

    struct BMP280Data
    {
        float pressure;
        float temperature;
    };

    BMP280Data getBMP280Data(unsigned iterations = 10)
    {
        Adafruit_BMP280 bmp;

        if (!bmp.begin(0x76))
        {
            throw SensorException("Could not access BMP280");
        }

        static float invalidValue = 73200.06f + 1;

        float pressure = 0;
        float temperature = 0;
        for (unsigned i = 0; i < iterations; i++)
        {
            // when the sensor has been turned of, it requires a "warm up phase" during which it may
            // provide a default value for a couple of iterations
            float curPressure = invalidValue;
            do {
                curPressure = bmp.readPressure();
            } while (curPressure < invalidValue);

            pressure += curPressure;
            temperature += bmp.readTemperature();
        }

        return {
            pressure / (100 * iterations),
            temperature / iterations
        };
    }

    float getMax44009Data()
    {
        Max44009 max44009(0x4A);  // default addr

        float illuminance = max44009.getLux();

        int error = max44009.getError();
        if (error != 0)
        {
            throw SensorException("Could not access Max44009");
        }

        return illuminance;
    }

    float getAudioDecibels(uint8_t microphone)
    {
        analogReadResolution(12);

        const int resolution = 4096;
        const float systemVoltage = 3.3f;

        // as a reference point, calculate the maximum value for RMS voltage
        const float halfResolution = (resolution / 2.0f);
        const float maxRawRms = sqrtf(halfResolution * halfResolution);
        const float maxRms = maxRawRms * (systemVoltage / resolution);

        unsigned long sumOfSquares = 0;

        unsigned long beginTime = millis();
        const unsigned long timeFrame = 50;

        unsigned long numSamples = 0;
        do {
            int audioValue = analogRead(microphone);

            audioValue = (audioValue - (resolution / 2));  // subtract the dc offset to center the signal at 0
            audioValue *= audioValue;               // square the value to remove negative values
            sumOfSquares += audioValue;         // sum the values
            numSamples++;
        } while (millis() - beginTime < timeFrame);

        // divide the sum of the squares to get the mean
        // and then take the square root of the mean
        const float rms = sqrtf(sumOfSquares / (float)numSamples);

        // convert the RMS value back into a voltage
        const float rmsVoltage = rms * (systemVoltage / resolution);

        // convert voltage into decibels
        const float db = 20 * log10f(rmsVoltage / maxRms);

        return db;
    }
}

Sensors::Data Sensors::getSensorData(const Configuration& config) const
{
    HTU21DData htu21dData = getHTU21DData();
    BMP280Data bmp280Data = getBMP280Data();
    float illuminance = getMax44009Data();
    float soundLevel = getAudioDecibels(config.microphone());

    return {
        bmp280Data.temperature,
        htu21dData.humidity,
        bmp280Data.pressure,
        illuminance,
        soundLevel};
}
}
