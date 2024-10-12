#pragma once

#include <WiFiManager.h>

#include <string>
#include <vector>

namespace envi_probe {
class Configuration {
public:
    Configuration();

    void load();
    void save();

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
    bool debugOutput() const;
    const MQTT& mqtt() const;
#ifdef HAS_DISPLAY
    const Display& display() const;
#endif

private:
    std::string m_apName;
    std::string m_deviceId{"envi_probe"};
    bool m_debugOutput{true};
    MQTT m_mqtt;
#ifdef HAS_DISPLAY
    Display m_display;
#endif

    WiFiManager m_wifiManager;
    WiFiManagerParameter m_deviceIdParam{"device_id", "Device ID",
                                         m_deviceId.c_str(), 40};
    WiFiManagerParameter m_brokerIpParam{"broker_ip", "Broker IP",
                                         m_mqtt.brokerIp.c_str(), 15};
    WiFiManagerParameter m_brokerPortParam{
        "broker_port", "Broker port", std::to_string(m_mqtt.brokerPort).c_str(),
        5};
    WiFiManagerParameter m_sendTimeParam{
        "send_time_seconds", "Send time (seconds)",
        std::to_string(m_mqtt.sendTimeSeconds).c_str(), 4};
#ifdef HAS_DISPLAY
    WiFiManagerParameter m_displayUpdateTime{
        "display_update_time", "Display update time (seconds)",
        std::to_string(m_display.updateTimeSeconds).c_str(), 4};
    WiFiManagerParameter m_displayRefreshTime{
        "display_refresh_time", "Display refresh time (seconds)",
        std::to_string(m_display.refreshTimeSeconds).c_str(), 4};
#endif
};
}  // namespace envi_probe
