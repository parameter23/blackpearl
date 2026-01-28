#ifndef ST7789_H
#define ST7789_H

#define CS_LOW()   gpio_clear(GPIOA, GPIO4)
#define CS_HIGH()  gpio_set(GPIOA, GPIO4)
#define DC_DATA()  gpio_set(GPIOA, GPIO3)
#define DC_CMD()   gpio_clear(GPIOA, GPIO3)

#include <libopencm3/stm32/gpio.h>
#include <stdint.h>

void st7789_init(void);
void st7789_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color);
void st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

#endif
