#ifndef DISPLAY_H
#define DISPLAY_H

#include <GxGDEH029A1/GxGDEH029A1.h>      // 2.9" b/w
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
    void end();

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

    GxIO_Class* m_io;
    GxEPD_Class* m_display;
};
}

#endif