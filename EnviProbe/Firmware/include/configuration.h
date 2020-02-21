#pragma once

#include <string>
#include <vector>

namespace envi_probe
{
class Configuration
{
public:
    Configuration();

    void load();
    void save();

    // pins
    uint8_t led() const;
    uint8_t microphone() const;

    // settings
    std::string deviceId() const;
    std::string mqttBroker() const;
    std::uint16_t mqttPort() const;
    std::string wifiSSID() const;
    std::string wifiPassword() const;
    std::vector<uint8_t>& bsecState();
    int sendTimeSeconds() const;
    int displayUpdateTimeSeconds() const;
    int displayRefreshTimeSeconds() const;
    bool debugOutput() const;

    void setDeviceId(std::string deviceId);
    void setMqttBroker(std::string mqttBroker);

private:
    std::string m_deviceId;
    std::string m_mqttBroker;
    std::string m_wifiSSID;
    std::string m_wifiPassword;
    std::vector<uint8_t> m_bsecState;
};
}
