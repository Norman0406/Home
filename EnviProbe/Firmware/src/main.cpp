#include <Wire.h>
#include <esp_task_wdt.h>

#include <functional>
#include <thread>

#include "configuration.h"
#include "data.h"
#include "exceptions.h"
#include "mqtt.h"
#include "wireless.h"

SET_LOOP_TASK_STACK_SIZE(16 * 1024);  // 16KB

#define BOOT_BUTTON 0
const unsigned long LONG_PRESS_SEC = 2;
const unsigned long VERY_LONG_PRESS_SEC = 15;
const unsigned int WDT_TIMEOUT_SEC = 30;
const unsigned int I2C_FREQUENCY = 200000;

#ifdef HAS_DISPLAY
#include "display.h"
#endif

#ifdef HAS_NEOPIXEL_LED
#define LED_PIN 21
#define LED_BRIGHTNESS 0.1
#endif

#ifdef HAS_LED
#define LED_PIN 2
#endif

#ifdef HAS_BME680
#include "bme680.h"
envi_probe::BME680 bme680;
#endif

#ifdef HAS_BMP280
#include "bmp280.h"
envi_probe::BMP280 bmp280;
#endif

#ifdef HAS_HTU21D
#include "htu21d.h"
envi_probe::HTU21D htu21d;
#endif

#ifdef HAS_SI7021
#include "si7021.h"
envi_probe::SI7021 si7021;
#endif

#ifdef HAS_HDC1080
#include "hdc1080.h"
envi_probe::HDC1080 hdc1080;
#endif

#ifdef HAS_SHT
#include "sht.h"
envi_probe::SHT sht;
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

std::thread processThread;

unsigned long lastSendTime = 0;

void setLed(float red_val, float green_val, float blue_val,
            bool override = false) {
    if (!config.ledEnabled() && !override) {
        return;
    }

#ifdef HAS_LED
    if (red_val > 0.5f || green_val > 0.5f || blue_val > 0.5f) {
        digitalWrite(LED_PIN, HIGH);
    } else {
        digitalWrite(LED_PIN, LOW);
    }
#endif
#ifdef HAS_NEOPIXEL_LED
    red_val *= LED_BRIGHTNESS;
    green_val *= LED_BRIGHTNESS;
    blue_val *= LED_BRIGHTNESS;
    neopixelWrite(LED_PIN, static_cast<uint8_t>(green_val * 255.0f),
                  static_cast<uint8_t>(red_val * 255.0f),
                  static_cast<uint8_t>(blue_val * 255.0f));
#endif
}

void process() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        wireless.process();
        mqtt.process();

        bool error = false;

#ifdef HAS_DISPLAY
        // don't update display too frequently
        unsigned long timeSinceLastUpdate = millis() - lastDisplayUpdate;
        if (timeSinceLastUpdate > config.display().updateTimeSeconds * 1e3) {
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
        if (timeSinceLastRefresh > config.display().refreshTimeSeconds * 1e3) {
            display.refresh();
            lastRefreshTime = millis();
        }

        display.setError(error);
#endif
    }
}

void bootBtnReleased();

unsigned long bootBtnPressedTime = 0;
bool resetRequested = false;

void bootBtnPressed() {
    // listen to another button press
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON), bootBtnReleased,
                    RISING);

    log_i(
        "Boot button pressed. Press >%ds s for config reset, >%ds for config "
        "and data reset.",
        LONG_PRESS_SEC, VERY_LONG_PRESS_SEC);

    // capture current time
    bootBtnPressedTime = millis();
}

void bootBtnReleased() {
    // attach another interrupt on button release
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON), bootBtnPressed,
                    FALLING);

    log_i("Boot button released");

    unsigned long bootBtnReleasedTime = millis();
    auto pressDuration = bootBtnReleasedTime - bootBtnPressedTime;

    // if it was a long button press, clear configuration and data
    if (pressDuration > LONG_PRESS_SEC * 1e3) {
        resetRequested = true;
        std::thread([pressDuration, config{std::reference_wrapper(config)},
                     data{std::reference_wrapper(data)}]() mutable {
            setLed(1, 0, 0);

            // clear config on normal long press
            config.get().clear();
            if (pressDuration > VERY_LONG_PRESS_SEC * 1e3) {
                // clear data on very long press
                data.get().clear();
            }

            setLed(0, 0, 0);
            ESP.restart();
        }).detach();
    }
}

void setupBootButton() {
    log_i("Setting up boot button");
    pinMode(BOOT_BUTTON, INPUT);

    // configure the boot button to format the internal storage on a long button
    // press
    attachInterrupt(digitalPinToInterrupt(BOOT_BUTTON), bootBtnPressed,
                    FALLING);
}

void setup() {
    Serial.begin(115200);

    delay(1000);

    log_i("===============================================");
    log_i("SDK: %s", ESP.getSdkVersion());
    log_i("Chip Revision: %u", ESP.getChipRevision());
    log_i("Flash Size: %.2f kB", ESP.getFlashChipSize() / 1024.0f);
    log_i("Sketch Size: %.2f kB / %.2f kB", ESP.getSketchSize() / 1024.0f,
          ESP.getFreeSketchSpace() / 1024.0f);
    log_i("Stack size: %.2f kB", getArduinoLoopTaskStackSize() / 1024.0f);
    log_i("===============================================");

    setupBootButton();

#ifdef HAS_DISPLAY
    // turn on voltage to sensors and display
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);
#endif

#ifdef HAS_LED
    pinMode(LED_PIN, OUTPUT);
#endif

    delay(5000);

    setLed(0, 0, 1);

    log_i("Initializing I2C (SDA: %d, SCL: %d, Frq: %d)", I2C_SDA, I2C_SCL,
          I2C_FREQUENCY);
    Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQUENCY);

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

        // configure watchdog timer
        log_i("Configuring watchdog timer with a timeout of %d seconds",
              WDT_TIMEOUT_SEC);
        esp_task_wdt_init(WDT_TIMEOUT_SEC,
                          true);  // enable panic so ESP32 restarts
        esp_task_wdt_add(NULL);   // add current thread to WDT watch

        // initialize sensors
#ifdef HAS_BME680
        bme680.begin(config, data);
#endif

#ifdef HAS_BMP280
        bmp280.begin(config);
#endif

#ifdef HAS_HTU21D
        htu21d.begin(config);
#endif

#ifdef HAS_SI7021
        si7021.begin(config);
#endif

#ifdef HAS_HDC1080
        hdc1080.begin(config);
#endif

#ifdef HAS_SHT
        sht.begin(config);
#endif

#ifdef HAS_MAX44009
        max44009.begin(config);
#endif

#ifdef HAS_MICROPHONE
        microphone.begin(config);
#endif

        // sensors are sensitive to timing, so everything that may change timing
        // happens in a different thread
        processThread = std::thread(process);

        // initialization phase is over
#ifdef HAS_DISPLAY
        display.setError(false);
        display.setActive(false);
#endif
    } catch (const envi_probe::Exception &e) {
        log_e("Exception: %s", e.what());

        setLed(1, 0, 0);

#ifdef HAS_DISPLAY
        display.setError(true);
#endif

        delay(1000);
        ESP.restart();
    }

    setLed(0, 0, 0, true);

    log_i("Setup finished");
}

#ifdef HAS_BME680
float temperatureOffset = 0;
#endif

void loop() {
    try {
        if (resetRequested) {
            return;
        }

        log_d("Resetting watchdog timer");
        esp_task_wdt_reset();

#ifdef HAS_BME680
        // set temperature offset to make BME680 readings more accurate
        bme680.setTemperatureOffset(temperatureOffset);
        auto bme680Data = bme680.read();

#ifdef HAS_DISPLAY
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            displayData.pressure = bme680Data.pressure / 100.0f;
        }
#endif
#endif

#ifdef HAS_BMP280
        envi_probe::BMP280::Data bmp280Data;
        bmp280Data = bmp280.read();
#endif

#ifdef HAS_SHT
        envi_probe::SHT::Data shtData;
        shtData = sht.read();

        // compute temperature offset for BME680
#ifdef HAS_BME680
        temperatureOffset = bme680Data.rawTemperature - shtData.temperature;
#endif

#ifdef HAS_DISPLAY
        {
            std::lock_guard<std::mutex> lock(dataMutex);
            displayData.temperature = shtData.temperature;
            displayData.humidity = shtData.humidity;
        }
#endif
#endif

#ifdef HAS_HTU21D
        envi_probe::HTU21D::Data htu21dData;
        htu21dData = htu21d.read();
#endif

#ifdef HAS_SI7021
        envi_probe::SI7021::Data si7021Data;
        si7021Data = si7021.read();
#endif

#ifdef HAS_HDC1080
        envi_probe::HDC1080::Data hdc1080Data;
        hdc1080Data = hdc1080.read();
#endif

#ifdef HAS_MAX44009
        envi_probe::Max44009::Illuminance max44009Data;
        max44009Data = max44009.read();
#endif

#ifdef HAS_MICROPHONE
        envi_probe::Microphone::Data microphoneData;
        microphoneData = microphone.read();
#endif

        if (!wireless.isConnected()) {
            setLed(1, 1, 0);
        } else if (!mqtt.isConnected()) {
            setLed(1, 0, 1);
        }

        // send data out every now and then
        unsigned long timeSinceLastSend = millis() - lastSendTime;
        if (timeSinceLastSend > config.mqtt().sendTimeSeconds * 1e3 &&
            wireless.isConnected() && mqtt.isConnected()) {
            lastSendTime = millis();
            log_d("Publishing data");

            setLed(0, 1, 0);

#ifdef HAS_DISPLAY
            display.setActive(true);
#endif

            mqtt.publish("time", lastSendTime);

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
            mqtt.publish("bme680/gasPercentage", bme680Data.gasPercentage);
            mqtt.publish("bme680/iaqAccuracy", bme680Data.iaqAccuracy);
            mqtt.publish("bme680/staticIaqAccuracy",
                         bme680Data.staticIaqAccuracy);
            mqtt.publish("bme680/co2Accuracy", bme680Data.co2Accuracy);
            mqtt.publish("bme680/breathVocAccuracy",
                         bme680Data.breathVocAccuracy);
            mqtt.publish("bme680/gasPercentageAcccuracy",
                         bme680Data.gasPercentageAcccuracy);
#endif

#ifdef HAS_BMP280
            mqtt.publish("bmp280/temperature", bmp280Data.temperature);
            mqtt.publish("bmp280/pressure", bmp280Data.pressure / 100.0f);
#endif

#ifdef HAS_HTU21D
            mqtt.publish("htu21d/temperature", htu21dData.temperature);
            mqtt.publish("htu21d/humidity", htu21dData.humidity);
#endif

#ifdef HAS_SI7021
            mqtt.publish("si7021/temperature", si7021Data.temperature);
            mqtt.publish("si7021/humidity", si7021Data.humidity);
#endif

#ifdef HAS_HDC1080
            mqtt.publish("hdc1080/temperature", hdc1080Data.temperature);
            mqtt.publish("hdc1080/humidity", hdc1080Data.humidity);
#endif

#ifdef HAS_SHT
            mqtt.publish("sht/temperature", shtData.temperature);
            mqtt.publish("sht/humidity", shtData.humidity);
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

#ifdef HAS_DISPLAY
            display.setActive(false);
#endif
        }
    } catch (const envi_probe::Exception &e) {
        log_e("Exception: %s", e.what());

        setLed(1, 0, 0);

#ifdef HAS_DISPLAY
        display.setError(true);
#endif

        delay(1000);
    } catch (...) {
        log_e("Unknown exception");

        setLed(1, 0, 0);

#ifdef HAS_DISPLAY
        display.setError(true);
#endif
        delay(1000);
    }

    setLed(0, 0, 0);

#ifdef HAS_NEOPIXEL_LED
    delay(20);
#endif

#ifdef HAS_DISPLAY
    // may still be on if exception during mqtt publishing
    display.setActive(false);
#endif
}
