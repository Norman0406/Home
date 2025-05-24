#include "mqtt.h"

#include <functional>
#include <memory>

#include "configuration.h"

namespace envi_probe {
MQTT::MQTT() {
    m_mqttClient.onConnect(
        std::bind(&MQTT::onConnected, this, std::placeholders::_1));
    m_mqttClient.onDisconnect(
        std::bind(&MQTT::onDisconnected, this, std::placeholders::_1));
    m_mqttClient.onPublish(
        std::bind(&MQTT::onPublished, this, std::placeholders::_1));
}

MQTT::~MQTT() {
    if (m_mqttClient.connected()) {
        m_mqttClient.disconnect();
    }
}

void MQTT::begin(const Configuration& config) {
    log_i("Initializing MQTT");

    m_deviceId = String(config.deviceId().c_str());
    m_server = String(config.mqtt().brokerIp.c_str());
    m_port = config.mqtt().brokerPort;

    IPAddress ipAddress;
    ipAddress.fromString(config.mqtt().brokerIp.c_str());

    m_mqttClient.setServer(ipAddress, config.mqtt().brokerPort);

    log_i("MQTT initialized");
}

void MQTT::connect() {
    log_i("Connecting to MQTT broker at %s: %d", m_server, m_port);

    m_reconnect = true;

    m_mqttClient.connect();
}

void MQTT::disconnect() {
    m_mqttClient.disconnect();

    m_reconnect = false;

    m_reconnectTimer.detach();
}

bool MQTT::isConnected() const { return m_connected; }

void MQTT::setTimeout(std::chrono::milliseconds timeout) {
    m_timeout = timeout;
}

void MQTT::process() {
    auto now = std::chrono::system_clock::now();

    std::lock_guard<std::mutex> lock(m_queueMutex);
    for (auto it = m_queuedPacketIds.begin(); it != m_queuedPacketIds.end();) {
        if (now - it->second > m_timeout) {
            log_w("Packet %d has not been acknowledged", it->first);

            it = m_queuedPacketIds.erase(it);
        } else {
            it++;
        }
    }
}

void MQTT::onConnected(bool sessionPresent) {
    log_i("MQTT client connected");

    m_connected = true;
}

void MQTT::connectToMqttStatic(MQTT* pThis) { pThis->m_mqttClient.connect(); }

void MQTT::onDisconnected(espMqttClientTypes::DisconnectReason reason) {
    if (m_connected) {
        log_i("MQTT client disconnected: %d", static_cast<int8_t>(reason));
    }

    m_connected = false;

    if (m_reconnect) {
        m_reconnectTimer.attach(2, connectToMqttStatic, this);
    }
}

void MQTT::onPublished(uint16_t packetId) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    if (m_queuedPacketIds.count(packetId)) {
        m_queuedPacketIds.erase(packetId);
    } else {
        log_w("Unknown packet acknowledged: %d", packetId);
    }
}
}  // namespace envi_probe
