#include "configuration.h"
#include "exceptions.h"
#include "configuration_wireless.inl"

#include <FS.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#define FORMAT_SPIFFS_IF_FAILED true

namespace envi_probe
{
const int startCountUntilRefresh = 10;

Configuration::Configuration()
    : m_wifiSSID{WirelessSsid}
    , m_wifiPassword{WirelessPassword}
{
}

void Configuration::load()
{
    if (debugOutput())
    {
        Serial.println("loading configuration");
    }

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
    {
        throw ConfigException("SPIFFS Mount Failed");
    }

    if (SPIFFS.exists("/config.json"))
    {
        File configFile = SPIFFS.open("/config.json", "r");

        if (!configFile)
        {
            throw ConfigException("Config file could not be opened");
        }

        if (debugOutput())
        {
            Serial.println("opened config file");
        }

        size_t size = configFile.size();

        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        size_t bytesRead = configFile.readBytes(buf.get(), size);

        if (bytesRead != size)
        {
            throw ConfigException("Could not read config file");
        }

        DynamicJsonDocument jsonDocument(4192);
        auto error = deserializeJson(jsonDocument, buf.get());

        if (!error)
        {
            JsonObject json = jsonDocument.as<JsonObject>();

            if (debugOutput())
            {
                serializeJson(jsonDocument, Serial);
                Serial.println();
            }

            m_mqttBroker = json["mqtt_broker"].as<const char*>();
            m_deviceId = json["device_id"].as<const char*>();

            JsonArray bsecState = json["bsec_state"].as<JsonArray>();
            for (auto value : bsecState)
            {
                m_bsecState.push_back(value.as<uint8_t>());
            }
        }
        else
        {
            throw ConfigException("failed to load json configuration");
        }

        configFile.close();
    }
}

void Configuration::save()
{
    if (debugOutput())
    {
        Serial.println("saving configuration");
    }

    DynamicJsonDocument jsonDocument(4192);
    jsonDocument["mqtt_broker"] = m_mqttBroker.c_str();
    jsonDocument["device_id"] = m_deviceId.c_str();

    JsonArray bsecState = jsonDocument["bsec_state"].to<JsonArray>();
    for (auto value : m_bsecState)
    {
        bsecState.add(value);
    }

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        throw ConfigException("failed to open config file for writing");
    }

    if (debugOutput())
    {
        serializeJson(jsonDocument, Serial);
        Serial.println();
    }

    serializeJson(jsonDocument, configFile);
    configFile.close();
}

uint8_t Configuration::led() const
{
    return LED_BUILTIN;
}

uint8_t Configuration::microphone() const
{
    return A0;
}

std::string Configuration::deviceId() const
{
    return m_deviceId;
}

std::string Configuration::mqttBroker() const
{
    return m_mqttBroker;
}

std::uint16_t Configuration::mqttPort() const
{
    return 1883;
}

std::string Configuration::wifiSSID() const
{
    return m_wifiSSID;
}

std::string Configuration::wifiPassword() const
{
    return m_wifiPassword;
}

std::vector<uint8_t>& Configuration::bsecState()
{
    return m_bsecState;
}

int Configuration::sendTimeSeconds() const
{
    return 30;
}

int Configuration::displayUpdateTimeSeconds() const
{
    return 1;
}

int Configuration::displayRefreshTimeSeconds() const
{
    return 600;
}

bool Configuration::debugOutput() const
{
    return true;
}

void Configuration::setDeviceId(std::string deviceId)
{
    m_deviceId = deviceId;
}

void Configuration::setMqttBroker(std::string mqttBroker)
{
    m_mqttBroker = mqttBroker;
}
}
