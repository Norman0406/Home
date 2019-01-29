#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxGDEW027W3/GxGDEW027W3.h>      // 2.7" b/w
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>

namespace envi_probe
{
class Configuration;

class Display
{
public:
    Display();
    virtual ~Display();

    void begin();
    void end(bool update = true);

    struct Data
    {
        float temperature;
        float humidity;
        float pressure;
    };

    void update(Data data);
    void setTime(String time);
    void setWifi(bool wifiOn);
    void setError(bool hasError);

private:
    void setActive(bool isActive);
    void drawTime(String time, int x, int y);
    void drawHumidity(float humidity, int x, int y);
    void drawTemperature(float temperature, int x, int y);
    void drawPressure(float pressure, int x, int y);

    GxIO_Class* m_io;
    GxEPD_Class* m_display;
};
}

#endif