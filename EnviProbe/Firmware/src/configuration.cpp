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

        JsonDocument jsonDocument;
        auto error = deserializeJson(jsonDocument, buf.get());

        if (!error) {
            JsonObject json = jsonDocument.as<JsonObject>();

            if (debugOutput()) {
                serializeJson(jsonDocument, Serial);
                Serial.println();
            }

            // general data
            m_deviceId = json["device_id"].as<const char*>();
            m_debugOutput = json["debug_output"].as<bool>();

            // MQTT
            m_mqtt.brokerIp = json["mqtt"]["broker_ip"].as<const char*>();
            m_mqtt.brokerPort = json["mqtt"]["broker_port"].as<std::uint16_t>();
            m_mqtt.sendTimeSeconds =
                json["mqtt"]["send_time_seconds"].as<int>();

            // WiFi
            m_wifi.ssid = json["wifi"]["ssid"].as<const char*>();
            m_wifi.password = json["wifi"]["password"].as<const char*>();

            if (json["display"].is<JsonObject>()) {
                m_display = Display{};
                m_display->updateTimeSeconds =
                    json["display"]["update_time_seconds"].as<int>();
                m_display->refreshTimeSeconds =
                    json["display"]["refresh_time_seconds"].as<int>();
            }
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

    JsonDocument jsonDocument;
    jsonDocument["device_id"] = m_deviceId.c_str();
    jsonDocument["debug_output"] = m_debugOutput;

    jsonDocument["mqtt"]["broker"] = m_mqtt.brokerIp.c_str();
    jsonDocument["mqtt"]["port"] = m_mqtt.brokerPort;
    jsonDocument["mqtt"]["send_time_seconds"] = m_mqtt.sendTimeSeconds;

    jsonDocument["wifi"]["ssid"] = m_wifi.ssid.c_str();
    jsonDocument["wifi"]["password"] = m_wifi.password.c_str();

    if (m_display) {
        jsonDocument["display"]["update_time_seconds"] =
            m_display->updateTimeSeconds;
        jsonDocument["display"]["refresh_time_seconds"] =
            m_display->refreshTimeSeconds;
    }

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

std::string Configuration::deviceId() const { return m_deviceId; }

bool Configuration::debugOutput() const { return m_debugOutput; }

const Configuration::WiFi& Configuration::wifi() const { return m_wifi; }

const Configuration::MQTT& Configuration::mqtt() const { return m_mqtt; }

const std::optional<Configuration::Display>& Configuration::display() const {
    return m_display;
}
}  // namespace envi_probe
