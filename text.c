#include "text.h"
#include "font6x8.h"
#include "st7789.h"
#include "framebuffer4.h"

void draw_char_fb_trans(int x, int y, char c, uint8_t color)
{
    if (c < 32 || c > 127)
        c = '?';

    const uint8_t *glyph = font6x8[c - 32];

    for (int col = 0; col < 6; col++) {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 8; row++) {
            if (bits & (1 << row))
                fb4_set_pixel(x + col, y + row, color);
        }
    }
}

void draw_text_fb_trans(int x, int y, const char *s, uint8_t color)
{
    while (*s) {
        draw_char_fb_trans(x, y, *s, color);
        x += 6;
        s++;
    }
}

void draw_char_fb(int x, int y, char c, uint8_t color, uint8_t bg)
{
    if (c < 32 || c > 127)
        c = '?';

    const uint8_t *glyph = font6x8[c - 32];

    for (int col = 0; col < 6; col++) {
        uint8_t bits = glyph[col];

        for (int row = 0; row < 8; row++) {
            uint8_t pixel = (bits & (1 << row)) ? color : bg;
            fb4_set_pixel(x + col, y + row, pixel);
        }
    }
}

void draw_text_fb(int x, int y, const char *s, uint8_t color, uint8_t bg)
{
    while (*s) {
        draw_char_fb(x, y, *s, color, bg);
        x += 6;  // feste Glyphenbreite
        s++;
    }
}

void draw_char(int x, int y, char c, uint16_t color, uint16_t bg)
{
    if (c < 32 || c > 127) c = '?';

    const uint8_t *glyph = font6x8[c - 32];

    for (int col = 0; col < 6; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < 8; row++) {
            uint16_t px = (bits & (1 << row)) ? color : bg;
            st7789_draw_pixel(x + col, y + row, px);
        }
    }
}

void draw_text(int x, int y, const char *s, uint16_t color, uint16_t bg)
{
    while (*s) {
        draw_char(x, y, *s, color, bg);
        x += 6;  /* fixed width */
        s++;
    }
}
