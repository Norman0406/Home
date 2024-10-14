#include "configuration.h"

#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

#include <sstream>

#include "exceptions.h"

#define FORMAT_SPIFFS_IF_FAILED true

namespace envi_probe {
const int startCountUntilRefresh = 10;

Configuration::Configuration() {
    std::stringstream ss;
    ss << "EnviProbe_" << std::hex << WIFI_getChipId();
    m_apName = ss.str();

    m_wifiManager.addParameter(&m_deviceIdParam);
    m_wifiManager.addParameter(&m_brokerIpParam);
    m_wifiManager.addParameter(&m_brokerPortParam);
    m_wifiManager.addParameter(&m_sendTimeParam);
#ifdef HAS_DISPLAY
    m_wifiManager.addParameter(&m_displayUpdateTime);
    m_wifiManager.addParameter(&m_displayRefreshTime);
#endif
    m_wifiManager.setSaveParamsCallback([this] {
        try {
            m_deviceId = m_deviceIdParam.getValue();
            m_mqtt.brokerIp = m_brokerIpParam.getValue();
            m_mqtt.brokerPort = std::stoi(m_brokerPortParam.getValue());
            m_mqtt.sendTimeSeconds = std::stoi(m_sendTimeParam.getValue());
#ifdef HAS_DISPLAY
            m_display.updateTimeSeconds =
                std::stoi(m_displayUpdateTime.getValue());
            m_display.refreshTimeSeconds =
                std::stoi(m_displayRefreshTime.getValue());
#endif

            save();
        } catch (...) {
            Serial.println("Unhandled exception, restarting");
            ESP.restart();
        }
    });
    m_wifiManager.setSaveConfigCallback([] { ESP.restart(); });
}

void Configuration::load() {
    if (debugOutput()) {
        Serial.println("Loading configuration");
    }

    try {
        if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
            throw ConfigException("SPIFFS Mount Failed");
        }

        if (!SPIFFS.exists(m_configFile.c_str())) {
            throw ConfigException("Configuration file does not exist");
        }

        File configFile = SPIFFS.open(m_configFile.c_str(), "r");

        if (!configFile) {
            throw ConfigException("Config file could not be opened");
        }

        if (debugOutput()) {
            Serial.println("Opened config file");
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
            m_deviceIdParam.setValue(m_deviceId.c_str(),
                                     m_deviceIdParam.getValueLength());
            m_debugOutput = json["debug_output"].as<bool>();

            // MQTT
            m_mqtt.brokerIp = json["mqtt"]["broker_ip"].as<const char*>();
            m_brokerIpParam.setValue(m_mqtt.brokerIp.c_str(),
                                     m_mqtt.brokerIp.length());
            m_mqtt.brokerPort = json["mqtt"]["broker_port"].as<std::uint16_t>();

            auto brokerPortStr = std::to_string(m_mqtt.brokerPort);
            m_brokerPortParam.setValue(brokerPortStr.c_str(),
                                       brokerPortStr.length());

            m_mqtt.sendTimeSeconds =
                json["mqtt"]["send_time_seconds"].as<int>();

            auto sendTimeSecondsStr = std::to_string(m_mqtt.sendTimeSeconds);
            m_sendTimeParam.setValue(sendTimeSecondsStr.c_str(),
                                     sendTimeSecondsStr.length());

#ifdef HAS_DISPLAY
            m_display.updateTimeSeconds =
                json["display"]["update_time_seconds"].as<int>();
            m_display.refreshTimeSeconds =
                json["display"]["refresh_time_seconds"].as<int>();
#endif
        } else {
            throw ConfigException("Failed to load json configuration");
        }

        configFile.close();
    } catch (const envi_probe::Exception& e) {
        Serial.println(
            "Could not open config file, starting configuration portal: " +
            String(e.what()));
        m_wifiManager.startConfigPortal(m_apName.c_str(), NULL);
    }

    if (!m_wifiManager.autoConnect(m_apName.c_str(), NULL)) {
        throw ConfigException("Could not connect");
    }

    if (debugOutput()) {
        Serial.println("Configuration finished");
    }
}

void Configuration::save() {
    if (debugOutput()) {
        Serial.println("Saving configuration");
    }

    JsonDocument jsonDocument = JsonObject();
    jsonDocument["device_id"] = m_deviceId.c_str();
    jsonDocument["debug_output"] = m_debugOutput;

    jsonDocument["mqtt"]["broker_ip"] = m_mqtt.brokerIp.c_str();
    jsonDocument["mqtt"]["broker_port"] = m_mqtt.brokerPort;
    jsonDocument["mqtt"]["send_time_seconds"] = m_mqtt.sendTimeSeconds;

#ifdef HAS_DISPLAY
    jsonDocument["display"]["update_time_seconds"] =
        m_display.updateTimeSeconds;
    jsonDocument["display"]["refresh_time_seconds"] =
        m_display.refreshTimeSeconds;
#endif

    File configFile = SPIFFS.open(m_configFile.c_str(), "w");
    if (!configFile) {
        throw ConfigException("Failed to open config file for writing");
    }

    if (debugOutput()) {
        serializeJson(jsonDocument, Serial);
        Serial.println();
    }

    serializeJson(jsonDocument, configFile);
    configFile.close();
}

void Configuration::clear() {
    Serial.println("Clearing configuration");

    SPIFFS.remove(m_configFile.c_str());
    m_wifiManager.resetSettings();

    Serial.println("Configuration cleared");
}

uint8_t Configuration::led() const { return LED_BUILTIN; }

std::string Configuration::deviceId() const { return m_deviceId; }

bool Configuration::debugOutput() const { return m_debugOutput; }

const Configuration::MQTT& Configuration::mqtt() const { return m_mqtt; }

#ifdef HAS_DISPLAY
const Configuration::Display& Configuration::display() const {
    return m_display;
}
#endif
}  // namespace envi_probe
