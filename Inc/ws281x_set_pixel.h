#ifndef __WS281X_SET_PIXEL_H
#define __WS281X_SET_PIXEL_H

#include "stm32_hal_ws281x.h"

void ws281x_set_pixel(uint16_t* buffer, uint16_t pin, uint8_t r, uint8_t g, uint8_t b, enum WS281x_ColorMode colorMode);

#endif//__WS281X_SET_PIXEL_H
