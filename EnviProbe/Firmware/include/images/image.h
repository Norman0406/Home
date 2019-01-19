#ifndef IMAGES_IMAGE_H
#define IMAGES_IMAGE_H

#include <stdint.h>

// icons from https://feathericons.com/

namespace envi_probe
{
typedef struct
{
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t dataSize;
} tImage;
}

#endif // IMAGE_H