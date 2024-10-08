#include "configuration.h"
#include "data.h"
#include "exceptions.h"
#include "mqtt.h"
#include "wireless.h"

// #include "hdc1080.h"
// #include "si7021.h"

#include <functional>
#include <thread>

#ifdef HAS_DISPLAY
#include "display.h"
#endif

#ifdef HAS_HTU21D
#include "htu21d.h"
envi_probe::HTU21D htu21d;
#endif

#ifdef HAS_BME680
#include "bme680.h"
envi_probe::BME680 bme680;
#endif

#ifdef HAS_BMP280
#include "bmp280.h"
envi_probe::BMP280 bmp280;
#endif

#ifdef HAS_SHT35D
#include "sht35d.h"
envi_probe::SHT35D sht35d;
#endif

#ifdef HAS_MAX44009
#include "max44009.h"
envi_probe::Max44009 max44009;
#endif

#ifdef HAS_MICROPHONE
#include "microphone.h"
envi_probe::Microphone microphone;
#endif

#ifdef HAS_DISPLAY
envi_probe::Display display;

std::mutex dataMutex;
envi_probe::Display::Data displayData = {"00:00", 0, 0, 0};

const char *ntpServer = "de.pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

namespace {
unsigned long lastDisplayUpdate = 0;
unsigned long lastRefreshTime = 0;

String getFormattedTime(const tm &timeInfo) {
    int hours = timeInfo.tm_hour;
    int minutes = timeInfo.tm_min;
    String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
    String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

    return hoursStr + ":" + minuteStr;
}
}  // namespace
#endif

envi_probe::Configuration config;
envi_probe::Data data{config};
envi_probe::Wireless wireless;
envi_probe::MQTT mqtt;
// envi_probe::HDC1080 hdc1080;
// envi_probe::Si7021 si7021;

std::thread processThread;

unsigned long lastSendTime = 0;

void process() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        wireless.process();
        mqtt.process();

        bool error = false;

#ifdef HAS_DISPLAY
        // don't update display too frequently
        unsigned long timeSinceLastUpdate = millis() - lastDisplayUpdate;
        if (timeSinceLastUpdate > config.display()->updateTimeSeconds * 1e3) {
            struct tm timeInfo;
            if (wireless.isConnected() && !getLocalTime(&timeInfo)) {
                error = true;
            }

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                displayData.time = getFormattedTime(timeInfo);
                display.update(displayData);
            }

            lastDisplayUpdate = millis();
        }

        // completely refresh the display from time to time
        unsigned long timeSinceLastRefresh = millis() - lastRefreshTime;
        if (timeSinceLastRefresh > config.display()->refreshTimeSeconds * 1e3) {
            display.refresh();
            lastRefreshTime = millis();
        }

        display.setError(error);
#endif
    }
}

void setup() {
#ifdef HAS_DISPLAY
    // turn on voltage to sensors and display
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);
#endif

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
#ifdef HAS_DISPLAY
        display.begin();
        display.setActive(true);
        display.setWifi(false);
#endif

        // load configuration
        config.load();

        // load data
        data.load();

#ifdef HAS_DISPLAY
        display.setConfig(config);
#endif

        std::reference_wrapper<envi_probe::MQTT> localMqtt(mqtt);
#ifdef HAS_DISPLAY
        std::reference_wrapper<envi_probe::Display> localDisplay(display);
#else
        void *localDisplay{nullptr};
#endif
        wireless.setConnectedHandler([localDisplay, localMqtt](bool connected) {
#ifdef HAS_DISPLAY
            localDisplay.get().setWifi(connected);
#endif

            if (connected) {
#ifdef HAS_DISPLAY
                configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
#endif
                localMqtt.get().connect();
            } else {
                localMqtt.get().disconnect();
            }
        });

        mqtt.begin(config);
        mqtt.setTimeout(std::chrono::seconds(30));

        wireless.begin(config);

        // initialize sensors
#ifdef HAS_BME680
        bme680.begin(config, data);
#endif

#ifdef HAS_BMP280
        bmp280.begin(config);
#endif

#ifdef HAS_SHT35D
        sht35d.begin(config);
#endif

#ifdef HAS_HTU21D
        htu21d.begin(config);
#endif

#ifdef HAS_MAX44009
        max44009.begin(config);
#endif

#ifdef HAS_MICROPHONE
        microphone.begin(config);
#endif

        // hdc1080.begin(config);
        // si7021.begin(config);

        // sensors are sensitive to timing, so everything that may change timing
        // happens in a different thread
        processThread = std::thread(process);

        if (config.debugOutput()) {
            Serial.println("Setup finished");
        }

        // initialization phase is over
#ifdef HAS_DISPLAY
        display.setError(false);
        display.setActive(false);
#endif
    } catch (const envi_probe::Exception &e) {
        Serial.println("Exception: " + String(e.what()));
#ifdef HAS_DISPLAY
        display.setError(true);
#endif
        delay(1000);
        ESP.restart();
    }
}

#ifdef HAS_BME680
float temperatureOffset = 0;
#endif

void loop() {
    try {
#ifdef HAS_BME680
        // set temperature offset to make BME680 readings more accurate
        bme680.setTemperatureOffset(temperatureOffset);
        auto bme680Data = bme680.read();

        if (config.display()) {
            std::lock_guard<std::mutex> lock(dataMutex);
            displayData.pressure = bme680Data.pressure / 100.0f;
        }
#endif

#ifdef HAS_BMP280
        envi_probe::BMP280::Data bmp280Data;
        bmp280Data = bmp280.read();
#endif

#ifdef HAS_SHT35D
        envi_probe::SHT35D::Data sht35dData;
        sht35dData = sht35d.read();

        // compute temperature offset for BME680
        if (config.bme680()) {
            temperatureOffset =
                bme680Data.rawTemperature - sht35dData.temperature;
        }

        if (config.display()) {
            std::lock_guard<std::mutex> lock(dataMutex);
            displayData.temperature = sht35dData.temperature;
            displayData.humidity = sht35dData.humidity;
        }
#endif

#ifdef HAS_HTU21D
        envi_probe::HTU21D::Data htu21dData;
        htu21dData = htu21d.read();
#endif

#ifdef HAS_MAX44009
        envi_probe::Max44009::Illuminance max44009Data;
        max44009Data = max44009.read();
#endif

#ifdef HAS_MICROPHONE
        envi_probe::Microphone::Data microphoneData;
        microphoneData = microphone.read();
#endif

        // auto hdc1080Data = m_hdc1080.read();
        // auto si7021Data = m_si7021.read();

        // send data out every now and then
        unsigned long timeSinceLastSend = millis() - lastSendTime;
        if (timeSinceLastSend > config.mqtt().sendTimeSeconds * 1e3 &&
            wireless.isConnected() && mqtt.isConnected()) {
#ifdef HAS_DISPLAY
            display.setActive(true);
#endif

#ifdef HAS_BME680
            mqtt.publish("bme680/iaq", bme680Data.iaq);
            mqtt.publish("bme680/rawTemperature", bme680Data.rawTemperature);
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
            mqtt.publish("bme680/compGasAccuracy", bme680Data.compGasAccuracy);
            mqtt.publish("bme680/gasPercentageAcccuracy",
                         bme680Data.gasPercentageAcccuracy);
#endif

#ifdef HAS_BMP280
            mqtt.publish("bmp280/temperature", bmp280Data.temperature);
            mqtt.publish("bmp280/pressure", bmp280Data.pressure / 100.0f);
#endif

#ifdef HAS_SHT35D
            mqtt.publish("sht35d/temperature", sht35dData.temperature);
            mqtt.publish("sht35d/humidity", sht35dData.humidity);
#endif

#ifdef HAS_HTU21D
            mqtt.publish("htu21d/temperature", htu21dData.temperature);
            mqtt.publish("htu21d/humidity", htu21dData.humidity);
#endif

#ifdef HAS_MAX44009
            mqtt.publish("max44009/illuminance", max44009Data);
#endif

#ifdef HAS_MICROPHONE
            if (microphoneData.isValid) {
                mqtt.publish("microphone/spl", microphoneData.spl);
                mqtt.publish("microphone/peak", microphoneData.peak);
            }
#endif

            // HDC1080
            // mqtt.publish("hdc1080/temperature", hdc1080Data.temperature);
            // mqtt.publish("hdc1080/humidity", hdc1080Data.humidity);

            // Si7021
            // mqtt.publish("si7021/temperature", si7021Data.temperature);
            // mqtt.publish("si7021/humidity", si7021Data.humidity);

#ifdef HAS_DISPLAY
            display.setActive(false);
#endif

            lastSendTime = millis();
        }
    } catch (const envi_probe::Exception &e) {
#ifdef HAS_DISPLAY
        display.setError(true);
#endif
        Serial.println("Exception: " + String(e.what()));
    } catch (...) {
#ifdef HAS_DISPLAY
        display.setError(true);
#endif
        Serial.println("Unknown exception");
    }

#ifdef HAS_DISPLAY
    // may still be on if exception during mqtt publishing
    display.setActive(false);
#endif
}
