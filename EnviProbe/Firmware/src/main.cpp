#include "mqtt.h"
#include "exceptions.h"
#include "wireless.h"
#include "configuration.h"
#include "display.h"
#include "htu21d.h"
#include "bme680.h"
#include "max44009.h"
#include "microphone.h"

#include <NTPClient.h>
#include <WiFiUdp.h>
#include <functional>
#include <thread>

envi_probe::Configuration config;
envi_probe::Wireless wireless;
envi_probe::Display display;
envi_probe::MQTT mqtt;
envi_probe::HTU21D m_htu21d;
envi_probe::BME680 m_bme680;
envi_probe::Max44009 m_max44009;
envi_probe::Microphone m_microphone;

std::thread m_processThread;

std::mutex m_dataMutex;
envi_probe::Display::Data displayData = {
    "00:00",
    0,
    0,
    0
};

namespace
{
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

    unsigned long lastDisplayUpdate = 0;
    unsigned long lastSendTime = 0;
    unsigned long lastRefreshTime = 0;

    String getFormattedTime(NTPClient& timeClient)
    {
        unsigned long rawTime = timeClient.getEpochTime();
        unsigned long hours = (rawTime % 86400L) / 3600;
        String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

        unsigned long minutes = (rawTime % 3600) / 60;
        String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

        return hoursStr + ":" + minuteStr;
    }

    void processThread()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));

            wireless.process();
            mqtt.process();

            bool error = false;

            // don't update display too frequently
            unsigned long timeSinceLastUpdate = millis() - lastDisplayUpdate;
            if (timeSinceLastUpdate > config.displayUpdateTimeSeconds() * 1e3)
            {
                if (wireless.isConnected() && !timeClient.update())
                {
                    error = true;
                }

                {
                    std::lock_guard<std::mutex> lock(m_dataMutex);
                    displayData.time = getFormattedTime(timeClient);
                    display.update(displayData);
                }

                lastDisplayUpdate = millis();
            }

            // completely refresh the display from time to time
            unsigned long timeSinceLastRefresh = millis() - lastRefreshTime;
            if (timeSinceLastRefresh > config.displayRefreshTimeSeconds() * 1e3)
            {
                display.refresh();
                lastRefreshTime = millis();
            }

            display.setError(error);
        }
    }
}

void setup()
{
    // turn on voltage to sensors and display
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);

    Serial.begin(9600);

    if (config.debugOutput())
    {
        Serial.println();
        Serial.println("SDK: " + String(ESP.getSdkVersion()));
        Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
    }

    try
    {
        display.begin();
        display.setActive(true);
        display.setWifi(false);

        // load configuration
        config.load();

        display.setConfig(config);

        std::reference_wrapper<envi_probe::MQTT> localMqtt(mqtt);
        std::reference_wrapper<envi_probe::Display> localDisplay(display);
        wireless.setConnectedHandler([localDisplay, localMqtt](bool connected)
        {
            localDisplay.get().setWifi(connected);

            if (connected)
            {
                localMqtt.get().connect();
            }
            else
            {
                localMqtt.get().disconnect();
            }
        });

        mqtt.begin(config);
        mqtt.setTimeout(std::chrono::seconds(1));

        wireless.begin(config);

        // initialize sensors
        m_htu21d.begin(config);
        m_bme680.begin(config);
        m_max44009.begin(config);
        m_microphone.begin(config);

        // sensors are sensitive to timing, so everything that may change timing happens in a different thread
        m_processThread = std::thread(processThread);

        display.setError(false);

        if (config.debugOutput())
        {
            Serial.println("Setup finished");
        }

        // initialization phase is over
        display.setActive(false);
    }
    catch (const envi_probe::Exception &e)
    {
        Serial.println("Exception: " + String(e.what()));
        display.setError(true);
        delay(1000);
        ESP.restart();
    }
}

float temperatureOffset = 0;

void loop()
{
    try
    {
        // set temperature offset to make BME680 readings more accurate
        m_bme680.setTemperatureOffset(temperatureOffset);

        // read sensor data
        auto bme680Data = m_bme680.read();
        auto htu21dData = m_htu21d.read();
        auto illuminance = m_max44009.read();
        auto decibels = m_microphone.read();

        // compute temperature offset
        temperatureOffset = bme680Data.rawTemperature - htu21dData.temperature;

        {
            std::lock_guard<std::mutex> lock(m_dataMutex);
            displayData.temperature = htu21dData.temperature;
            displayData.humidity = htu21dData.humidity;
            displayData.pressure = bme680Data.pressure / 100.0f;
        }

        // send data out every now and then
        unsigned long timeSinceLastSend = millis() - lastSendTime;
        if (timeSinceLastSend > config.sendTimeSeconds() * 1e3 && wireless.isConnected() && mqtt.isConnected())
        {
            display.setActive(true);

            // HTU21D
            mqtt.publish("htu21d/temperature", htu21dData.temperature);
            mqtt.publish("htu21d/humidity", htu21dData.humidity);

            // BME680
            mqtt.publish("bme680/iaq", bme680Data.iaq);
            mqtt.publish("bme680/rawTemperature", bme680Data.rawTemperature);
            mqtt.publish("bme680/pressure", bme680Data.pressure);
            mqtt.publish("bme680/rawHumidity", bme680Data.rawHumidity);
            mqtt.publish("bme680/gasResistance", bme680Data.gasResistance);
            mqtt.publish("bme680/stabStatus", bme680Data.stabStatus);
            mqtt.publish("bme680/runInStatus", bme680Data.runInStatus);
            mqtt.publish("bme680/temperature", bme680Data.temperature);
            mqtt.publish("bme680/humidity", bme680Data.humidity);
            mqtt.publish("bme680/staticIaq", bme680Data.staticIaq);
            mqtt.publish("bme680/co2Equivalent", bme680Data.co2Equivalent);
            mqtt.publish("bme680/breathVocEquivalent", bme680Data.breathVocEquivalent);
            mqtt.publish("bme680/compGasValue", bme680Data.compGasValue);
            mqtt.publish("bme680/gasPercentage", bme680Data.gasPercentage);
            mqtt.publish("bme680/iaqAccuracy", bme680Data.iaqAccuracy);
            mqtt.publish("bme680/staticIaqAccuracy", bme680Data.staticIaqAccuracy);
            mqtt.publish("bme680/co2Accuracy", bme680Data.co2Accuracy);
            mqtt.publish("bme680/breathVocAccuracy", bme680Data.breathVocAccuracy);
            mqtt.publish("bme680/compGasAccuracy", bme680Data.compGasAccuracy);
            mqtt.publish("bme680/gasPercentageAcccuracy", bme680Data.gasPercentageAcccuracy);

            // MAX44009
            mqtt.publish("max44009/illuminance", illuminance);

            // Microphone
            mqtt.publish("microphone/decibels", decibels);

            // combined data
            mqtt.publish("temperature", htu21dData.temperature);
            mqtt.publish("humidity", htu21dData.humidity);
            mqtt.publish("pressure", bme680Data.pressure / 100.0f);
            mqtt.publish("indoorAirQuality", bme680Data.iaq);
            mqtt.publish("illuminance", illuminance);
            mqtt.publish("sound", decibels);

            display.setActive(false);

            lastSendTime = millis();
        }
    }
    catch (const envi_probe::Exception &e)
    {
        display.setError(true);
        Serial.println("Exception: " + String(e.what()));
    }
    catch (...)
    {
        display.setError(true);
    }

    // may still be on if exception during mqtt publishing
    display.setActive(false);
}
