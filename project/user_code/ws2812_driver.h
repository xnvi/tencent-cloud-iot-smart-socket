#ifndef __WS2812_DRIVER_H
#define __WS2812_DRIVER_H

#include <stdint.h>

void ws2812_init(void);
void ws2812_reset(void);
void ws2812_write_byte(uint8_t dat);
void ws2812_write_color_rgb(uint8_t *color);
void ws2812_write_color_u32(uint32_t color);

#endif
