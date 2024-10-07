#include "bme680.h"
#include "bmp280.h"
#include "configuration.h"
#include "data.h"
#include "display.h"
#include "exceptions.h"
#include "htu21d.h"
#include "max44009.h"
#include "microphone.h"
#include "mqtt.h"
#include "sht35d.h"
#include "wireless.h"

// #include "hdc1080.h"
// #include "si7021.h"

#include <functional>
#include <thread>

envi_probe::Configuration config;
envi_probe::Data data{config};
envi_probe::Wireless wireless;
envi_probe::Display display;
envi_probe::MQTT mqtt;
envi_probe::BME680 m_bme680;
envi_probe::BMP280 m_bmp280;
envi_probe::SHT35D m_sht35d;
envi_probe::HTU21D m_htu21d;
envi_probe::Max44009 m_max44009;
envi_probe::Microphone m_microphone;
// envi_probe::HDC1080 m_hdc1080;
// envi_probe::Si7021 m_si7021;

std::thread m_processThread;

std::mutex m_dataMutex;
envi_probe::Display::Data displayData = {"00:00", 0, 0, 0};

const char *ntpServer = "de.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

namespace {
unsigned long lastDisplayUpdate = 0;
unsigned long lastSendTime = 0;
unsigned long lastRefreshTime = 0;

String getFormattedTime(const tm &timeInfo) {
    int hours = timeInfo.tm_hour;
    int minutes = timeInfo.tm_min;
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    return hoursStr + ":" + minuteStr;
}

void processThread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        wireless.process();
        mqtt.process();

        bool error = false;

        // don't update display too frequently
        if (config.display()) {
            unsigned long timeSinceLastUpdate = millis() - lastDisplayUpdate;
            if (timeSinceLastUpdate >
                config.display()->updateTimeSeconds * 1e3) {
                struct tm timeInfo;
                if (wireless.isConnected() && !getLocalTime(&timeInfo)) {
                    error = true;
                }

                {
                    std::lock_guard<std::mutex> lock(m_dataMutex);
                    displayData.time = getFormattedTime(timeInfo);
                    display.update(displayData);
                }

                lastDisplayUpdate = millis();
            }

            // completely refresh the display from time to time
            unsigned long timeSinceLastRefresh = millis() - lastRefreshTime;
            if (timeSinceLastRefresh >
                config.display()->refreshTimeSeconds * 1e3) {
                display.refresh();
                lastRefreshTime = millis();
            }

            display.setError(error);
        }
    }
}
}  // namespace

void setup() {
    // turn on voltage to sensors and display
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);

    Serial.begin(9600);

    if (config.debugOutput()) {
        Serial.println();
        Serial.println("===============================================");
        Serial.println("SDK: " + String(ESP.getSdkVersion()));
        Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
        Serial.println(
            "Flash Size: " + String(ESP.getFlashChipSize() / 1024.0f) + " kB");
        Serial.println("Sketch Size: " + String(ESP.getSketchSize() / 1024.0f) +
                       " kb / " + String(ESP.getFreeSketchSpace() / 1024.0f) +
                       " kb");
        Serial.println("===============================================");
    }

    try {
        if (config.display()) {
            display.begin();
            display.setActive(true);
            display.setWifi(false);
        }

        // load configuration
        config.load();

        // load data
        data.load();

        if (config.display()) {
            display.setConfig(config);
        }

        std::reference_wrapper<envi_probe::MQTT> localMqtt(mqtt);
        std::reference_wrapper<envi_probe::Display> localDisplay(display);
        wireless.setConnectedHandler([localDisplay, localMqtt](bool connected) {
            if (config.display()) {
                localDisplay.get().setWifi(connected);
            }

            if (connected) {
                configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                localMqtt.get().connect();
            } else {
                localMqtt.get().disconnect();
            }
        });

        mqtt.begin(config);
        mqtt.setTimeout(std::chrono::seconds(30));

        wireless.begin(config);

        // initialize sensors
        if (config.bme680()) {
            m_bme680.begin(config, data);
        }

        if (config.bmp280()) {
            m_bmp280.begin(config);
        }

        if (config.sht35d()) {
            m_sht35d.begin(config);
        }

        if (config.htu21d()) {
            m_htu21d.begin(config);
        }

        if (config.max44009()) {
            m_max44009.begin(config);
        }

        if (config.microphone()) {
            m_microphone.begin(config);
        }

        // m_hdc1080.begin(config);
        // m_si7021.begin(config);

        // sensors are sensitive to timing, so everything that may change timing
        // happens in a different thread
        m_processThread = std::thread(processThread);

        if (config.debugOutput()) {
            Serial.println("Setup finished");
        }

        // initialization phase is over
        if (config.display()) {
            display.setError(false);
            display.setActive(false);
        }
    } catch (const envi_probe::Exception &e) {
        Serial.println("Exception: " + String(e.what()));
        if (config.display()) {
            display.setError(true);
        }
        delay(1000);
        ESP.restart();
    }
}

float temperatureOffset = 0;

void loop() {
    try {
        envi_probe::BME680::Data bme680Data;
        if (config.bme680()) {
            // set temperature offset to make BME680 readings more accurate
            m_bme680.setTemperatureOffset(temperatureOffset);
            bme680Data = m_bme680.read();

            if (config.display()) {
                std::lock_guard<std::mutex> lock(m_dataMutex);
                displayData.pressure = bme680Data.pressure / 100.0f;
            }
        }

        envi_probe::BMP280::Data bmp280Data;
        if (config.bmp280()) {
            bmp280Data = m_bmp280.read();
        }

        envi_probe::SHT35D::Data sht35dData;
        if (config.sht35d()) {
            sht35dData = m_sht35d.read();

            // compute temperature offset for BME680
            if (config.bme680()) {
                temperatureOffset =
                    bme680Data.rawTemperature - sht35dData.temperature;
            }

            if (config.display()) {
                std::lock_guard<std::mutex> lock(m_dataMutex);
                displayData.temperature = sht35dData.temperature;
                displayData.humidity = sht35dData.humidity;
            }
        }

        envi_probe::HTU21D::Data htu21dData;
        if (config.htu21d()) {
            htu21dData = m_htu21d.read();
        }

        envi_probe::Max44009::Illuminance max44009Data;
        if (config.max44009()) {
            max44009Data = m_max44009.read();
        }

        envi_probe::Microphone::Data microphoneData;
        if (config.microphone()) {
            microphoneData = m_microphone.read();
        }

        // auto hdc1080Data = m_hdc1080.read();
        // auto si7021Data = m_si7021.read();

        // send data out every now and then
        unsigned long timeSinceLastSend = millis() - lastSendTime;
        if (timeSinceLastSend > config.mqtt().sendTimeSeconds * 1e3 &&
            wireless.isConnected() && mqtt.isConnected()) {
            if (config.display()) {
                display.setActive(true);
            }

            if (config.bme680()) {
                mqtt.publish("bme680/iaq", bme680Data.iaq);
                mqtt.publish("bme680/rawTemperature",
                             bme680Data.rawTemperature);
                mqtt.publish("bme680/pressure", bme680Data.pressure / 100.0f);
                mqtt.publish("bme680/rawHumidity", bme680Data.rawHumidity);
                mqtt.publish("bme680/gasResistance", bme680Data.gasResistance);
                mqtt.publish("bme680/stabStatus", bme680Data.stabStatus);
                mqtt.publish("bme680/runInStatus", bme680Data.runInStatus);
                mqtt.publish("bme680/temperature", bme680Data.temperature);
                mqtt.publish("bme680/humidity", bme680Data.humidity);
                mqtt.publish("bme680/staticIaq", bme680Data.staticIaq);
                mqtt.publish("bme680/co2Equivalent", bme680Data.co2Equivalent);
                mqtt.publish("bme680/breathVocEquivalent",
                             bme680Data.breathVocEquivalent);
                mqtt.publish("bme680/compGasValue", bme680Data.compGasValue);
                mqtt.publish("bme680/gasPercentage", bme680Data.gasPercentage);
                mqtt.publish("bme680/iaqAccuracy", bme680Data.iaqAccuracy);
                mqtt.publish("bme680/staticIaqAccuracy",
                             bme680Data.staticIaqAccuracy);
                mqtt.publish("bme680/co2Accuracy", bme680Data.co2Accuracy);
                mqtt.publish("bme680/breathVocAccuracy",
                             bme680Data.breathVocAccuracy);
                mqtt.publish("bme680/compGasAccuracy",
                             bme680Data.compGasAccuracy);
                mqtt.publish("bme680/gasPercentageAcccuracy",
                             bme680Data.gasPercentageAcccuracy);
            }

            if (config.bmp280()) {
                mqtt.publish("bmp280/temperature", bmp280Data.temperature);
                mqtt.publish("bmp280/pressure", bmp280Data.pressure / 100.0f);
            }

            if (config.sht35d()) {
                mqtt.publish("sht35d/temperature", sht35dData.temperature);
                mqtt.publish("sht35d/humidity", sht35dData.humidity);
            }

            if (config.htu21d()) {
                mqtt.publish("htu21d/temperature", htu21dData.temperature);
                mqtt.publish("htu21d/humidity", htu21dData.humidity);
            }

            if (config.max44009()) {
                mqtt.publish("max44009/illuminance", max44009Data);
            }

            if (config.microphone()) {
                if (microphoneData.isValid) {
                    mqtt.publish("microphone/spl", microphoneData.spl);
                    mqtt.publish("microphone/peak", microphoneData.peak);
                }
            }

            // HDC1080
            // mqtt.publish("hdc1080/temperature", hdc1080Data.temperature);
            // mqtt.publish("hdc1080/humidity", hdc1080Data.humidity);

            // Si7021
            // mqtt.publish("si7021/temperature", si7021Data.temperature);
            // mqtt.publish("si7021/humidity", si7021Data.humidity);

            if (config.display()) {
                display.setActive(false);
            }

            lastSendTime = millis();
        }
    } catch (const envi_probe::Exception &e) {
        if (config.display()) {
            display.setError(true);
        }
        Serial.println("Exception: " + String(e.what()));
    } catch (...) {
        if (config.display()) {
            display.setError(true);
        }
        Serial.println("Unknown exception");
    }

    // may still be on if exception during mqtt publishing
    display.setActive(false);
}
