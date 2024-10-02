#pragma once

#include <string>
#include <vector>

namespace envi_probe {
class Configuration {
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
    int sendTimeSeconds() const;
    int displayUpdateTimeSeconds() const;
    int displayRefreshTimeSeconds() const;
    bool debugOutput() const;

private:
    std::string m_deviceId{"envi_probe"};
    std::string m_mqttBroker{"192.168.1.10"};
    std::uint16_t m_mqttPort{1883};
    std::string m_wifiSSID{""};
    std::string m_wifiPassword{""};
    int m_sendTimeSeconds{30};
    int m_displayUpdateTimeSeconds{1};
    int m_displayRefreshTimeSeconds{600};
    bool m_debugOutput{true};
};
}  // namespace envi_probe
