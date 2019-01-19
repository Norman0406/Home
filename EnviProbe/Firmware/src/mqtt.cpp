#include <mqtt.h>
#include <configuration.h>

#include <memory>

namespace envi_probe
{
MQTT::MQTT(const Configuration& config)
    : m_deviceId(config.deviceId().c_str())
    , m_mqttClient{Adafruit_MQTT_Client(&m_client, config.mqttBroker().c_str(), config.mqttPort(), config.deviceId().c_str(), "", "")}
    , m_debugOutput{config.debugOutput()}
{
    if (m_debugOutput)
    {
        Serial.println("Connecting to MQTT broker at " +
            String(config.mqttBroker().c_str()) + ":" +
            String(config.mqttPort()));
    }

    connect();
}

void MQTT::connect()
{
    int8_t ret;

    // Stop if already connected.
    if (m_mqttClient.connected())
    {
        return;
    }

    uint8_t retries = 3;
    while ((ret = m_mqttClient.connect()) != 0) // connect will return 0 for connected
    {
        if (m_debugOutput)
        {
            Serial.println(m_mqttClient.connectErrorString(ret));
            Serial.println("Retrying MQTT connection in 5 seconds...");
        }

        m_mqttClient.disconnect();
        delay(5000); // wait 5 seconds
        retries--;
        if (retries == 0)
        {
            return;
        }
    }

    m_connected = true;
}

void MQTT::process()
{
    delay(200);
}

bool MQTT::isConnected() const
{
    return m_connected;
}
}
