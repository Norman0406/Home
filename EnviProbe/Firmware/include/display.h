#pragma once

#include <GxGDEW027W3/GxGDEW027W3.h>  // 2.7" b/w
#include <GxIO/GxIO.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>

#include <mutex>

namespace envi_probe {
class Configuration;

class Display {
public:
    Display();
    virtual ~Display();

    void begin();
    void end(bool update = true);

    struct Data {
        String time;
        float temperature;
        float humidity;
        float pressure;
    };

    void refresh();
    void setConfig(const Configuration& config);
    void update(Data data);
    void setActive(bool isActive);
    void setWifi(bool wifiOn);
    void setError(bool hasError);

private:
    void setTime(String time);
    void printHeader(String deviceId);
    void drawTime(String time, int x, int y);
    void drawHumidity(float humidity, int x, int y);
    void drawTemperature(float temperature, int x, int y);
    void drawPressure(float pressure, int x, int y);

    std::mutex m_mutex;
    GxIO_Class* m_io;
    GxEPD_Class* m_display;

    bool m_isActive{false};
    bool m_wifiOn{false};
    bool m_hasError{false};
};
}  // namespace envi_probe
