#pragma once

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#include <chrono>
#include <mutex>
#include <unordered_map>

#include "exceptions.h"

namespace envi_probe {
class Configuration;

namespace {
template <typename T>
String to_string(T value);

template <>
String to_string(float value) {
    return String(value, 8);
}

template <>
String to_string(double value) {
    return String(value, 8);
}

template <>
String to_string(uint8_t value) {
    return String(value);
}

template <>
String to_string(int value) {
    return String(value);
}
}  // namespace

class MQTT {
public:
    using string = String;

    MQTT();
    ~MQTT();

    template <typename T>
    void publish(string topic, T value) {
        string topicStr = "environmental/" + m_deviceId + "/" + topic;
        string valueStr = to_string(value);

        auto packetId =
            m_mqttClient.publish(topicStr.c_str(), 1, false, valueStr.c_str());

        if (packetId == 0) {
            // not connected
            throw NetworkException("Could not publish");
        }

        if (m_debugOutput) {
            Serial.println("Published " + topicStr + ": " + valueStr);
        }

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_queuedPacketIds[packetId] = std::chrono::system_clock::now();
        }
    }

    void begin(const Configuration &config);
    void connect();
    void disconnect();

    bool isConnected() const;

    void setTimeout(std::chrono::milliseconds timeout);
    void process();

private:
    void onConnected(bool sessionPresent);
    void onDisconnected(AsyncMqttClientDisconnectReason reason);
    void onPublished(uint16_t packetId);
    static void connectToMqttStatic(MQTT *pThis);

    std::mutex m_queueMutex;
    std::unordered_map<uint16_t, std::chrono::system_clock::time_point>
        m_queuedPacketIds;
    std::chrono::milliseconds m_timeout{std::chrono::seconds(1)};
    AsyncMqttClient m_mqttClient;
    string m_deviceId;
    bool m_debugOutput{false};
    bool m_reconnect{false};
    bool m_connected{false};
    bool m_debug{false};
    string m_server;
    std::uint16_t m_port;
    Ticker m_reconnectTimer;
};
}  // namespace envi_probe
