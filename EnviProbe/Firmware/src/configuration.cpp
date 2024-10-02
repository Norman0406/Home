#include "configuration.h"

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include "exceptions.h"

#define FORMAT_SPIFFS_IF_FAILED true

namespace envi_probe {
const int startCountUntilRefresh = 10;

Configuration::Configuration() {}

void Configuration::load() {
    if (debugOutput()) {
        Serial.println("loading configuration");
    }

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
        throw ConfigException("SPIFFS Mount Failed");
    }

    if (SPIFFS.exists("/config.json")) {
        File configFile = SPIFFS.open("/config.json", "r");

        if (!configFile) {
            throw ConfigException("Config file could not be opened");
        }

        if (debugOutput()) {
            Serial.println("opened config file");
        }

        size_t size = configFile.size();

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        size_t bytesRead = configFile.readBytes(buf.get(), size);

        if (bytesRead != size) {
            throw ConfigException("Could not read config file");
        }

        DynamicJsonDocument jsonDocument(4192);
        auto error = deserializeJson(jsonDocument, buf.get());

        if (!error) {
            JsonObject json = jsonDocument.as<JsonObject>();

            if (debugOutput()) {
                serializeJson(jsonDocument, Serial);
                Serial.println();
            }

            m_mqttBroker = json["mqtt_broker"].as<const char *>();
            m_mqttPort = json["mqtt_port"].as<std::uint16_t>();
            m_deviceId = json["device_id"].as<const char *>();
            m_wifiSSID = json["wifi_ssid"].as<const char *>();
            m_wifiPassword = json["wifi_password"].as<const char *>();
            m_sendTimeSeconds = json["send_time_seconds"].as<int>();
            m_displayUpdateTimeSeconds =
                json["display_update_time_seconds"].as<int>();
            m_displayRefreshTimeSeconds =
                json["display_refresh_time_seconds"].as<int>();
            m_debugOutput = json["debug_output"].as<bool>();
        } else {
            throw ConfigException("failed to load json configuration");
        }

        configFile.close();
    }
}

void Configuration::save() {
    if (debugOutput()) {
        Serial.println("saving configuration");
    }

    DynamicJsonDocument jsonDocument(4192);
    jsonDocument["mqtt_broker"] = m_mqttBroker.c_str();
    jsonDocument["mqtt_port"] = m_mqttPort;
    jsonDocument["device_id"] = m_deviceId.c_str();
    jsonDocument["wifi_ssid"] = m_wifiSSID.c_str();
    jsonDocument["wifi_password"] = m_wifiPassword.c_str();
    jsonDocument["send_time_seconds"] = m_sendTimeSeconds;
    jsonDocument["display_update_time_seconds"] = m_displayUpdateTimeSeconds;
    jsonDocument["display_refresh_time_seconds"] = m_displayRefreshTimeSeconds;
    jsonDocument["debug_output"] = m_debugOutput;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        throw ConfigException("failed to open config file for writing");
    }

    if (debugOutput()) {
        serializeJson(jsonDocument, Serial);
        Serial.println();
    }

    serializeJson(jsonDocument, configFile);
    configFile.close();
}

uint8_t Configuration::led() const { return LED_BUILTIN; }

uint8_t Configuration::microphone() const { return A0; }

std::string Configuration::deviceId() const { return m_deviceId; }

std::string Configuration::mqttBroker() const { return m_mqttBroker; }

std::uint16_t Configuration::mqttPort() const { return m_mqttPort; }

std::string Configuration::wifiSSID() const { return m_wifiSSID; }

std::string Configuration::wifiPassword() const { return m_wifiPassword; }

int Configuration::sendTimeSeconds() const { return m_sendTimeSeconds; }

int Configuration::displayUpdateTimeSeconds() const {
    return m_displayUpdateTimeSeconds;
}

int Configuration::displayRefreshTimeSeconds() const {
    return m_displayRefreshTimeSeconds;
}

bool Configuration::debugOutput() const { return m_debugOutput; }
}  // namespace envi_probe
