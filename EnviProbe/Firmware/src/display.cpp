#include "display.h"
#include "configuration.h"

#include "images/refresh.h"
#include "images/alert.h"
#include "images/arrow_up.h"
#include "images/arrow_down.h"
#include "images/wifi.h"
#include "images/wifi_off.h"

#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMono24pt7b.h>

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

namespace envi_probe
{
Display::Display()
    : m_io{nullptr}
    , m_display{nullptr}
{
}

Display::~Display()
{
    delete m_io;
    delete m_display;
}

void Display::begin()
{
    Serial.println("Initializing display");

    m_io = new GxIO_Class(SPI, SS, 17, 16);
    m_display = new GxEPD_Class(*m_io, 16, 4);

    m_display->init();

    m_display->setTextColor(GxEPD_BLACK);
    m_display->setRotation(3);



    setActive(true);
}

void Display::update(Data data)
{
    {
        m_display->setFont(&FreeMonoBold24pt7b);

        // temperature
        m_display->setCursor(10, 36);
        m_display->println(data.temperature, 1);

        // humidity
        m_display->setCursor(10, 78);
        m_display->println(data.humidity, 1);

        // pressure
        m_display->setCursor(10, 120);
        m_display->println(data.pressure, 1);
    }

    {
        const auto& font = FreeMono12pt7b;
        m_display->setFont(&font);

        // temperature
        m_display->drawCircle(130, 26, 3, GxEPD_BLACK);
        m_display->setCursor(135, 36);
        m_display->println("C");

        // humidity
        m_display->setCursor(125, 78);
        m_display->println("%");

        // pressure
        int pressurePos = 183;
        if (data.pressure < 1000)
        {
            pressurePos = 155;
        }
        m_display->setCursor(pressurePos, 120);
        m_display->println("hPa");
    }
}

void Display::setTime(String time)
{
    m_display->setFont(&FreeMonoBold24pt7b);

    // time
    //m_display->setCursor(10, 50);
    //m_display->println(time);
}

void Display::setWifi(bool wifiOn)
{
    if (wifiOn)
    {
        m_display->drawBitmap(
            m_display->width() - wifi.width - 1,
            0,
            wifi.data,
            wifi.width,
            wifi.height,
            GxEPD_BLACK);
    }
    else
    {
        m_display->drawBitmap(
            m_display->width() - wifi_off.width - 1,
            0,
            wifi_off.data,
            wifi_off.width,
            wifi_off.height,
            GxEPD_BLACK);
    }
}

void Display::setError(bool hasError)
{
    if (hasError)
    {
        m_display->drawBitmap(
            m_display->width() - alert.width - 1,
            wifi.height,
            alert.data,
            alert.width,
            alert.height,
            GxEPD_BLACK);
    }
    else
    {
        m_display->fillRect(
            m_display->width() - alert.width - 1,
            wifi.height,
            alert.width,
            alert.height,
            GxEPD_WHITE);
    }

    m_display->updateWindow(
        m_display->width() - alert.width - 2,
        wifi.height,
        wifi.width,
        wifi.height);
}

void Display::setActive(bool isActive)
{
    if (isActive)
    {
        m_display->drawBitmap(
            m_display->width() - refresh.width - 1,
            m_display->height() - refresh.height,
            refresh.data,
            refresh.width,
            refresh.height,
            GxEPD_BLACK);
    }
    else
    {
        m_display->fillRect(
            m_display->width() - refresh.width - 1,
            m_display->height() - refresh.height,
            refresh.width,
            refresh.height,
            GxEPD_WHITE);
    }

    m_display->updateWindow(
        m_display->width() - refresh.width - 2,
        m_display->height() - refresh.height - 1,
        refresh.width,
        refresh.height);
}

void Display::end()
{
    setActive(false);

    // Screen seems to have a double buffer. Without the second update, a previous image will be
    // shown on next start up.
    m_display->update();
    m_display->update();

    m_display->powerDown();
}
}
