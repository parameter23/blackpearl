#ifndef FRAMEBUFFER4_H
#define FRAMEBUFFER4_H
#include <string.h> // für memcpy
#include <stdint.h>

#define FB4_WIDTH   320
#define FB4_HEIGHT  240

#define FB4_SIZE    ((FB4_WIDTH * FB4_HEIGHT) / 2)

// Framebuffer (2 Pixel pro Byte)
extern uint8_t framebuffer4[FB4_SIZE];

// Laufzeit-Palette (RGB565)
extern uint16_t fb4_palette[16];

// Default-Palette (wird beim Init nach fb4_palette kopiert)
static const uint16_t fb4_default_palette[16] = {
    0x0000, // 0 schwarz
    0xFFFF, // 1 weiß
    0xF800, // 2 rot
    0x07E0, // 3 grün
    0x001F, // 4 blau
    0xFFE0, // 5 gelb
    0xF81F, // 6 magenta
    0x07FF, // 7 cyan
    0x8410, // 8 grau
    0x4208, // 9 dunkelgrau
    0xFC00, // 10 orange
    0x83E0, // 11 oliv
    0x7BEF, // 12 hellgrau
    0xA145, // 13 braun
    0xFD20, // 14 gold
    0xFFFF  // 15 weiß (alias)
};

void fb4_clear(uint8_t color);
void fb4_set_pixel(int x, int y, uint8_t color);
uint8_t fb4_get_pixel(int x, int y);

void fb4_flush(void);
void fb4_init_palette(void);
void fb4_flush_dma(void);    // NEU

void fb4_fill_rect(int x, int y, int w, int h, uint8_t color);

#endif
