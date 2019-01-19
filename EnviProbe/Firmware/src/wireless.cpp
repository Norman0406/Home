#include <wireless.h>
#include <exceptions.h>
#include <configuration.h>

#include <WiFi.h>

//#define USE_WIFIMANAGER

#ifdef USE_WIFIMANAGER
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

namespace envi_probe
{
namespace
{
// flag for saving data
bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback()
{
    shouldSaveConfig = true;
}
} // namespace
} // namespace envi_probe
#endif

namespace envi_probe
{
Wireless::Wireless()
{
}

void Wireless::connect(Configuration &config)
{
    // setup wifi connection
#ifdef USE_WIFIMANAGER
    AsyncWebServer server(80);
    DNSServer dns;

    AsyncWiFiManager wifiManager(&server, &dns);

    char mqtt_broker[15];
    char device_id[32];

    strcpy(mqtt_broker, config.mqttBroker().c_str());
    strcpy(device_id, config.deviceId().c_str());

    wifiManager.setConnectTimeout(30);
    //wifiManager.setConfigPortalTimeout(60);
    wifiManager.setDebugOutput(config.debugOutput());
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    // //wifiManager.resetSettings();
    AsyncWiFiManagerParameter custom_mqtt_broker("mqtt_broker", "MQTT Broker", mqtt_broker, 15);
    AsyncWiFiManagerParameter custom_device_id("device_id", "Device ID", device_id, 32);
    wifiManager.addParameter(&custom_mqtt_broker);
    wifiManager.addParameter(&custom_device_id);

    if (!wifiManager.autoConnect("EnviProbe"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(5000);
        return;
    }

    if (config.debugOutput())
    {
        Serial.println("Connected");
    }

    //read updated parameters
    config.setMqttBroker(custom_mqtt_broker.getValue());
    config.setDeviceId(custom_device_id.getValue());

    if (config.debugOutput())
    {
        String ip = ::WiFi.localIP().toString();
        Serial.println("IP Address: " + ip);
        Serial.println("Device ID: " + String(custom_device_id.getValue()));
    }

    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
        config.save();
    }
#else
    ::WiFi.disconnect();
    ::WiFi.enableSTA(true);
    ::WiFi.enableAP(false);
    delay(500);

    ::WiFi.setAutoReconnect(true);
    ::WiFi.begin(config.wifiSSID().c_str(), config.wifiPassword().c_str());

    if (config.debugOutput())
    {
        Serial.println("Connecting");
    }

    auto status = ::WiFi.waitForConnectResult();
    if (status != WL_CONNECTED)
    {
        throw WifiException("Couldn't connect to wifi");
    }

    if (config.debugOutput())
    {
        Serial.print("IP address: ");
        Serial.println(::WiFi.localIP());
    }
#endif
}
} // namespace envi_probe
