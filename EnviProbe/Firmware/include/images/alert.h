#ifndef IMAGES_ALERT_H
#define IMAGES_ALERT_H

#include "image.h"

namespace envi_probe
{
static const uint8_t PROGMEM image_data_alert[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x66, 0x00, 0x00,
    0xc3, 0x00, 0x00, 0xc3, 0x00, 0x01, 0x81, 0x80, 0x03, 0x99, 0xc0, 0x03, 0x18, 0xc0, 0x06, 0x18,
    0x60, 0x06, 0x18, 0x60, 0x0c, 0x18, 0x30, 0x1c, 0x18, 0x38, 0x18, 0x00, 0x18, 0x30, 0x00, 0x0c,
    0x30, 0x18, 0x0c, 0x60, 0x18, 0x06, 0x60, 0x00, 0x06, 0x60, 0x00, 0x06, 0x7f, 0xff, 0xfe, 0x3f,
    0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const tImage alert = {image_data_alert, 24, 24, 8};
}

#endif // ALERT_H