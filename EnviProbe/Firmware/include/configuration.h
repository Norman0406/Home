#pragma once

#include <optional>
#include <string>
#include <vector>

namespace envi_probe {
class Configuration {
public:
    Configuration();

    void load();
    void save();

    struct WiFi {
        std::string ssid{""};
        std::string password{""};
    };

    struct MQTT {
        std::string brokerIp{"192.168.1.10"};
        std::uint16_t brokerPort{1883};
        int sendTimeSeconds{30};
    };

    struct Display {
        int updateTimeSeconds{1};
        int refreshTimeSeconds{600};
    };

    // settings
    uint8_t led() const;
    std::string deviceId() const;
    int sendTimeSeconds() const;
    bool debugOutput() const;
    const WiFi& wifi() const;
    const MQTT& mqtt() const;
    const std::optional<Display>& display() const;

private:
    std::string m_deviceId{"envi_probe"};
    bool m_debugOutput{true};
    WiFi m_wifi;
    MQTT m_mqtt;
    std::optional<Display> m_display;
};
}  // namespace envi_probe
