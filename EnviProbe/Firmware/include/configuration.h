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

    struct BME680 {};

    struct BMP280 {};

    struct SHT35D {};

    struct HTU21D {};

    struct Max44009 {};

    struct Microphone {
        uint8_t pin;
    };

    // settings
    uint8_t led() const;
    std::string deviceId() const;
    int sendTimeSeconds() const;
    bool debugOutput() const;
    const WiFi& wifi() const;
    const MQTT& mqtt() const;
    const std::optional<Display>& display() const;
    const std::optional<BME680>& bme680() const;
    const std::optional<BMP280>& bmp280() const;
    const std::optional<SHT35D>& sht35d() const;
    const std::optional<HTU21D>& htu21d() const;
    const std::optional<Max44009>& max44009() const;
    const std::optional<Microphone>& microphone() const;

private:
    std::string m_deviceId{"envi_probe"};
    bool m_debugOutput{true};
    WiFi m_wifi;
    MQTT m_mqtt;
    std::optional<Display> m_display;
    std::optional<BME680> m_bme680;
    std::optional<BMP280> m_bmp280;
    std::optional<SHT35D> m_sht35d;
    std::optional<HTU21D> m_htu21d;
    std::optional<Max44009> m_max44009;
    std::optional<Microphone> m_microphone;
};
}  // namespace envi_probe
