#include <configuration.h>
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
        Serial.println("SPIFFS Mount Failed");
        return;
    }

    if (SPIFFS.exists("/config.json"))
    {
        File configFile = SPIFFS.open("/config.json", "r");

        if (configFile)
        {
            if (debugOutput())
            {
                Serial.println("opened config file");
            }

            size_t size = configFile.size();

            // Allocate a buffer to store contents of the file.
            std::unique_ptr<char[]> buf(new char[size]);

            configFile.readBytes(buf.get(), size);
            DynamicJsonBuffer jsonBuffer;
            JsonObject &json = jsonBuffer.parseObject(buf.get());

            if (debugOutput())
            {
                json.printTo(Serial);
                Serial.println();
            }

            if (json.success())
            {
                m_mqttBroker = json["mqtt_broker"].as<const char*>();
                m_deviceId = json["device_id"].as<const char*>();
            }
            else
            {
                Serial.println("failed to load json config");
            }

            configFile.close();
        }
    }
}

void Configuration::save()
{
    if (debugOutput())
    {
        Serial.println("saving configuration");
    }

    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
    json["mqtt_broker"] = m_mqttBroker.c_str();
    json["device_id"] = m_deviceId.c_str();

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }

    if (debugOutput())
    {
        json.printTo(Serial);
    }

    json.printTo(configFile);
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

int Configuration::sleepTimeSeconds() const
{
    return 30;
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
