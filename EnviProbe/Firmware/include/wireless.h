#pragma once

#include <esp_event_legacy.h>

#include <functional>

namespace envi_probe {
class Configuration;

class Wireless {
public:
    using IsConnectedHandler = std::function<void(bool)>;

    Wireless();

    void begin(Configuration &config);
    void process();
    bool isConnected() const;

    void setConnectedHandler(IsConnectedHandler isConnectedHandler);

private:
    void onConnected(bool isConnected);

    IsConnectedHandler m_isConnectedHandler;
    bool m_isConnected{false};
    bool m_debugOutput{false};
};
}  // namespace envi_probe
