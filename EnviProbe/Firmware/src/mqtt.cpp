#include "mqtt.h"
#include "configuration.h"

#include <memory>
#include <functional>

namespace envi_probe
{
MQTT::MQTT()
{
    m_mqttClient.onConnect(std::bind(&MQTT::onConnected, this, std::placeholders::_1));
    m_mqttClient.onDisconnect(std::bind(&MQTT::onDisconnected, this, std::placeholders::_1));
    m_mqttClient.onPublish(std::bind(&MQTT::onPublished, this, std::placeholders::_1));
}

MQTT::~MQTT()
{
    if (m_mqttClient.connected())
    {
        m_mqttClient.disconnect();
    }
}

void MQTT::begin(const Configuration &config)
{
    m_deviceId = String(config.deviceId().c_str());
    m_debugOutput = config.debugOutput();
    m_server = String(config.mqttBroker().c_str());
    m_port = config.mqttPort();

    IPAddress ipAddress;
    ipAddress.fromString(config.mqttBroker().c_str());

    m_mqttClient.setServer(ipAddress, config.mqttPort());
}

void MQTT::connect()
{
    if (m_debugOutput)
    {
        Serial.println("Connecting to MQTT broker at " + m_server + ":" + m_port);
    }

    m_reconnect = true;

    m_mqttClient.connect();
}

void MQTT::disconnect()
{
    m_mqttClient.disconnect();

    m_reconnect = false;

    m_reconnectTimer.detach();
}

bool MQTT::isConnected() const
{
    return m_connected;
}

void MQTT::setTimeout(std::chrono::milliseconds timeout)
{
    m_timeout = timeout;
}

void MQTT::process()
{
    auto now = std::chrono::system_clock::now();

    std::lock_guard<std::mutex> lock(m_queueMutex);
    for (auto it = m_queuedPacketIds.begin(); it != m_queuedPacketIds.end();)
    {
        if (now - it->second > m_timeout)
        {
            if (m_debugOutput)
            {
                Serial.printf("Packet %d has not been acknowledged\n", it->first);
            }

            it = m_queuedPacketIds.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void MQTT::onConnected(bool sessionPresent)
{
    if (m_debugOutput)
    {
        Serial.println("MQTT client connected");
    }

    m_connected = true;
}

void MQTT::connectToMqttStatic(MQTT* pThis)
{
    pThis->m_mqttClient.connect();
}

void MQTT::onDisconnected(AsyncMqttClientDisconnectReason reason)
{
    if (m_debugOutput && m_connected)
    {
        Serial.printf("MQTT client disconnected: %d\n", static_cast<int8_t>(reason));
    }

    m_connected = false;

    if (m_reconnect)
    {
        m_reconnectTimer.attach(2, connectToMqttStatic, this);
    }
}

void MQTT::onPublished(uint16_t packetId)
{
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_queuedPacketIds.count(packetId))
    {
        m_queuedPacketIds.erase(packetId);
    }
    else if (m_debugOutput)
    {
        Serial.printf("Unknown packet acknowledged: %d\n", packetId);
    }
}
}
