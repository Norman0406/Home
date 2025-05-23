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
    m_wifiManager.addParameter(&m_ledEnabledParam);
    m_wifiManager.setSaveParamsCallback([this] {
        try {
            m_deviceId = m_deviceIdParam.getValue();
            m_ledEnabled = (strncmp(m_ledEnabledParam.getValue(), "T", 1) == 0);
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
            log_e("Unhandled exception, restarting");
            ESP.restart();
        }
    });
    m_wifiManager.setSaveConfigCallback([] { ESP.restart(); });
}

void Configuration::load() {
    log_i("Loading configuration from %s", m_configFile.c_str());

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

        log_i("Opened config file");

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

            String serializedJson;
            serializeJson(jsonDocument, serializedJson);
            log_i("Json: %s", serializedJson.c_str());

            // general data
            m_deviceId = json["device_id"].as<const char*>();
            m_deviceIdParam.setValue(m_deviceId.c_str(),
                                     m_deviceIdParam.getValueLength());

            m_ledEnabled = json["led_enabled"].as<bool>();
            if (m_ledEnabled) {
                m_ledEnabledParam.setValue("T", 1);
            } else {
                m_ledEnabledParam.setValue("", 0);
            }

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
        log_e("Could not open config file, starting configuration portal: %s",
              e.what());
        m_wifiManager.startConfigPortal(m_apName.c_str(), NULL);
    }

    if (!m_wifiManager.autoConnect(m_apName.c_str(), NULL)) {
        throw ConfigException("Could not connect");
    }

    log_i("Configuration finished");
}

void Configuration::save() {
    log_i("Saving configuration to %s", m_configFile.c_str());

    JsonDocument jsonDocument = JsonObject();
    jsonDocument["device_id"] = m_deviceId.c_str();
    jsonDocument["led_enabled"] = m_ledEnabled;

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

    String serializedJson;
    serializeJson(jsonDocument, serializedJson);
    log_i("Json: %s", serializedJson.c_str());

    serializeJson(jsonDocument, configFile);
    configFile.close();

    log_i("Configuration saved");
}

void Configuration::clear() {
    log_i("Clearing configuration");

    SPIFFS.remove(m_configFile.c_str());
    m_wifiManager.resetSettings();

    log_i("Configuration cleared");
}

std::string Configuration::deviceId() const { return m_deviceId; }

bool Configuration::ledEnabled() const { return m_ledEnabled; }

const Configuration::MQTT& Configuration::mqtt() const { return m_mqtt; }

#ifdef HAS_DISPLAY
const Configuration::Display& Configuration::display() const {
    return m_display;
}
#endif
}  // namespace envi_probe
