#include "wireless.h"

#include <WiFi.h>

#include "configuration.h"
#include "exceptions.h"

namespace envi_probe {
Wireless::Wireless() {}

void Wireless::begin(Configuration &config) {
    m_debugOutput = config.debugOutput();

    ::WiFi.disconnect();
    ::WiFi.enableSTA(true);
    ::WiFi.enableAP(false);
    delay(500);

    ::WiFi.setAutoReconnect(true);

    ::WiFi.begin(config.wifi().ssid.c_str(), config.wifi().password.c_str());
}

void Wireless::onConnected(bool isConnected) {
    if (m_isConnected == isConnected) {
        return;
    }

    m_isConnected = isConnected;

    if (isConnected) {
        if (m_debugOutput) {
            Serial.println("Wifi connected to " + WiFi.localIP().toString());
        }
    } else {
        if (m_debugOutput) {
            Serial.println("Wifi disconnected");
        }
    }

    m_isConnectedHandler(isConnected);
}

void Wireless::process() {
    wl_status_t status = WiFi.status();

    onConnected(status == WL_CONNECTED);

    if (status != WL_CONNECTED) {
        ::WiFi.reconnect();
        status = static_cast<wl_status_t>(::WiFi.waitForConnectResult());
    }

    onConnected(status == WL_CONNECTED);
}

bool Wireless::isConnected() const { return m_isConnected; }

void Wireless::setConnectedHandler(
    Wireless::IsConnectedHandler isConnectedHandler) {
    m_isConnectedHandler = isConnectedHandler;
}
}  // namespace envi_probe
