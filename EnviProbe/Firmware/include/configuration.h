#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

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
    int sleepTimeSeconds() const;
    bool debugOutput() const;

    void setDeviceId(std::string deviceId);
    void setMqttBroker(std::string mqttBroker);

private:
    std::string m_deviceId;
    std::string m_mqttBroker;
    std::string m_wifiSSID;
    std::string m_wifiPassword;
};
}

#endif // CONFIGURATION_H