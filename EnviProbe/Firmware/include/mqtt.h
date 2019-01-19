#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

namespace envi_probe
{
class Configuration;

namespace
{
    template <typename T>
    String to_string(T value);

    template <>
    String to_string(float value)
    {
        return String(value, 8);
    }
}

class MQTT
{
public:
    using string = String;

    MQTT(const Configuration &config);

    template <typename T>
    void publish(string topic, T value)
    {
        string topicStr = "environmental/" + m_deviceId + "/" + topic;
        string valueStr = to_string(value);
        m_mqttClient.publish(topicStr.c_str(), valueStr.c_str());

        if (m_debugOutput)
        {
            Serial.println(topicStr + ": " + valueStr);
        }
    }

    void process();
    bool isConnected() const;

private:
    void connect();

    string m_deviceId;
    WiFiClient m_client;
    Adafruit_MQTT_Client m_mqttClient;
    bool m_debugOutput{false};
    bool m_connected{false};
};
}

#endif // MQTT_H
