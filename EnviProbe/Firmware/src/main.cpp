#include <mqtt.h>
#include <sensors.h>
#include <exceptions.h>
#include <wireless.h>
#include <configuration.h>
#include <display.h>

#include <NTPClient.h>

unsigned long startTime = 0;

// if this variable is set when the device enters the loop, it will not sleep but immediately
// restart and try again
bool restartOnError = false;

envi_probe::Configuration config;
envi_probe::Display display;

namespace
{
    String getFormattedTime(NTPClient& timeClient)
    {
        unsigned long rawTime = timeClient.getEpochTime();
        unsigned long hours = (rawTime % 86400L) / 3600;
        String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

        unsigned long minutes = (rawTime % 3600) / 60;
        String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

        return hoursStr + ":" + minuteStr;
    }
}

void setup()
{
    startTime = millis();

    display.begin();

    // turn on voltage to sensors and display
    pinMode(25, OUTPUT);
    digitalWrite(25, HIGH);

    Serial.begin(9600);

    if (config.debugOutput())
    {
        Serial.println();
        Serial.println("SDK: " + String(ESP.getSdkVersion()));
        Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
    }

    try
    {
        // load configuration
        config.load();

        // get sensor readings before connecting to wifi, since they may take some time
        envi_probe::Sensors sensors;
        auto sensorData = sensors.getSensorData(config);

        envi_probe::Display::Data data {
            sensorData.temperature,
            sensorData.humidity,
            sensorData.pressure
        };

        display.update(data);

        // connect to wifi
        envi_probe::Wireless wireless;
        wireless.connect(config);

        display.setWifi(true);

        // get time
        WiFiUDP ntpUDP;
        NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
        timeClient.begin();

        int updateAttempts = 3;
        while (!timeClient.update())
        {
            updateAttempts--;
            delay(200);
        }

        if (updateAttempts == 0)
        {
            display.setError(true);
        }
        else
        {
            display.setTime(getFormattedTime(timeClient));
        }

        // connect to MQTT
        envi_probe::MQTT mqtt(config);
        if (!mqtt.isConnected())
        {
            throw envi_probe::NetworkException("Could not connect to MQTT broker");
        }

        // publish sensor data
        mqtt.publish("temperature", sensorData.temperature);
        mqtt.publish("humidity", sensorData.humidity);
        mqtt.publish("pressure", sensorData.pressure);
        mqtt.publish("illuminance", sensorData.illuminance);
        mqtt.publish("sound", sensorData.soundLevel);

        mqtt.process();

        display.setError(false);
    }
    catch (const envi_probe::WifiException &e)
    {
        display.setWifi(false);
        Serial.println("Exception: " + String(e.what()));
        restartOnError = e.restart();
        return;
    }
    catch (const envi_probe::Exception &e)
    {
        display.setError(true);
        Serial.println("Exception: " + String(e.what()));
        restartOnError = e.restart();
        return;
    }
}

void loop()
{
    // setup function has finished, complete final steps and go into deep sleep

    if (restartOnError)
    {
        if (config.debugOutput())
        {
            Serial.println("Restarting device");
        }

        delay(500);
        Serial.end();

        // restart() does not properly reset the I2C bus
        //ESP.restart();

        display.end(false);

        // turn off voltage to sensors and display
        digitalWrite(25, LOW);

        ESP.deepSleep(1);
    }
    else
    {
        // the device should send data every x seconds, so calculate the necessary sleep time by taking
        // into considering the total computation time for the algorithms
        long sleepTime = config.sleepTimeSeconds() * 1e6; // 30 seconds
        unsigned long execTime = millis() - startTime;
        sleepTime -= (execTime * 1e3);
        if (sleepTime < 0)
        {
            sleepTime = 0;
        }

        if (config.debugOutput())
        {
            const float sleepTimeSec = sleepTime * 1e-6;
            Serial.print("Sleeping for " + String(sleepTimeSec) + " seconds");
        }

        delay(500);
        Serial.end();

        display.end();

        // turn off voltage to sensors and display
        digitalWrite(25, LOW);

        ESP.deepSleep(sleepTime);
    }
}
