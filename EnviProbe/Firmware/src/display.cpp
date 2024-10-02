#include "display.h"

#include <Fonts/FreeMono12pt7b.h>      // large units
#include <Fonts/FreeMono9pt7b.h>       // small units
#include <Fonts/FreeMonoBold18pt7b.h>  // small values
#include <Fonts/FreeMonoBold24pt7b.h>  // large values
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/Org_01.h>  // captions

#include "configuration.h"
#include "images/alert.h"
#include "images/arrow_down.h"
#include "images/arrow_up.h"
#include "images/refresh.h"
#include "images/wifi.h"
#include "images/wifi_off.h"

namespace envi_probe {
Display::Display() : m_io{nullptr}, m_display{nullptr} {}

Display::~Display() {
    delete m_io;
    delete m_display;
}

void Display::begin() {
    m_io = new GxIO_Class(SPI, SS, 17, 16);
    m_display = new GxEPD_Class(*m_io, 16, 4);

    m_display->init();

    m_display->setTextColor(GxEPD_BLACK);
    m_display->setRotation(1);

    // draw a line
    int separatorPosition = wifi.height;
    m_display->drawLine(0, separatorPosition, m_display->width(),
                        separatorPosition, GxEPD_BLACK);
    m_display->update();
}

void Display::setConfig(const Configuration &config) {
    printHeader(config.deviceId().c_str());
}

void Display::printHeader(String deviceId) {
    m_display->setFont(&FreeSans9pt7b);

    uint16_t textWidth = 0;
    uint16_t textHeight = 0;
    {
        int16_t x1 = 0, y1 = 0;
        m_display->getTextBounds(deviceId, 0, 0, &x1, &y1, &textWidth,
                                 &textHeight);
    }

    const int xPosition = (m_display->width() / 2) - (textWidth / 2);
    const int yPosition = (wifi.height / 2) + (textHeight / 2);

    m_display->setCursor(xPosition, yPosition);
    m_display->println(deviceId);

    m_display->updateWindow(0, 0, m_display->width(), wifi.height);
}

void Display::drawTime(String time, int x, int y) {
    m_display->setFont(&Org_01);
    m_display->setCursor(x, y);
    m_display->println("Time");

    m_display->setFont(&FreeMonoBold24pt7b);
    m_display->setCursor(x, y + 37);
    m_display->println(time);
}

void Display::drawHumidity(float humidity, int x, int y) {
    m_display->setFont(&Org_01);
    m_display->setCursor(x, y);
    m_display->println("Humidity");

    m_display->setFont(&FreeMonoBold24pt7b);
    m_display->setCursor(x, y + 37);
    m_display->println(humidity, 1);

    m_display->setFont(&FreeMono12pt7b);
    m_display->setCursor(x + 113, y + 23);
    m_display->println("%");
}

void Display::drawTemperature(float temperature, int x, int y) {
    m_display->setFont(&Org_01);
    m_display->setCursor(x, y);
    m_display->println("Temperature");

    m_display->setFont(&FreeMonoBold18pt7b);
    m_display->setCursor(x, y + 30);
    m_display->println(temperature, 1);

    m_display->setFont(&FreeMono9pt7b);
    m_display->drawCircle(x + 90, y + 10, 3, GxEPD_BLACK);
    m_display->setCursor(x + 95, y + 20);
    m_display->println("C");
}

void Display::drawPressure(float pressure, int x, int y) {
    m_display->setFont(&Org_01);
    m_display->setCursor(x, y);
    m_display->println("Pressure");

    m_display->setFont(&FreeMonoBold18pt7b);
    m_display->setCursor(x, y + 30);
    m_display->println((int)pressure);

    int pressurePos = x + 88;
    if (pressure < 1000) {
        pressurePos -= 20;
    }

    m_display->setFont(&FreeMono9pt7b);
    m_display->setCursor(pressurePos, y + 20);
    m_display->println("hPa");
}

void Display::update(Data data) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_display->fillRect(0, 80, m_display->width(), m_display->height() - 81,
                        GxEPD_WHITE);

    drawHumidity(data.humidity, 75, 87);
    drawTemperature(data.temperature, 10, 140);
    drawPressure(data.pressure, 135, 140);

    m_display->updateWindow(0, 80, m_display->width(),
                            m_display->height() - 81);

    setTime(data.time);
}

void Display::refresh() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_display->update();
}

void Display::setTime(String time) {
    m_display->fillRect(0, 27, m_display->width(), 52, GxEPD_WHITE);

    drawTime(time, 65, 35);

    m_display->updateWindow(0, 27, m_display->width(), 52);
}

void Display::setWifi(bool wifiOn) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (wifiOn) {
        if (!m_wifiOn) {
            m_display->fillRect(0, 0, wifi.width, wifi.height, GxEPD_WHITE);
        }

        m_display->drawBitmap(0, 0, wifi.data, wifi.width, wifi.height,
                              GxEPD_BLACK);
    } else {
        m_display->drawBitmap(0, 0, wifi_off.data, wifi_off.width,
                              wifi_off.height, GxEPD_BLACK);
    }

    m_display->updateWindow(0, 0, wifi.width, wifi.height);

    m_wifiOn = wifiOn;
}

void Display::setError(bool hasError) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_hasError == hasError) {
        return;
    }

    if (hasError) {
        m_display->drawBitmap(wifi.width, 0, alert.data, alert.width,
                              alert.height, GxEPD_BLACK);
    } else {
        m_display->fillRect(wifi.width, 0, alert.width, alert.height,
                            GxEPD_WHITE);
    }

    m_display->updateWindow(wifi.width, 0, alert.width, alert.height);

    m_hasError = hasError;
}

void Display::setActive(bool isActive) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_isActive == isActive) {
        return;
    }

    if (isActive) {
        m_display->drawBitmap(
            m_display->width() - envi_probe::refresh.width - 1, 0,
            envi_probe::refresh.data, envi_probe::refresh.width,
            envi_probe::refresh.height, GxEPD_BLACK);
    } else {
        m_display->fillRect(m_display->width() - envi_probe::refresh.width - 1,
                            0, envi_probe::refresh.width,
                            envi_probe::refresh.height, GxEPD_WHITE);
    }

    m_display->updateWindow(m_display->width() - envi_probe::refresh.width - 1,
                            0, envi_probe::refresh.width,
                            envi_probe::refresh.height);

    m_isActive = isActive;
}

void Display::end(bool update) {
    if (update) {
        m_display->update();
    }

    m_display->powerDown();
}
}  // namespace envi_probe
