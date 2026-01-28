#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>

void draw_char(int x, int y, char c, uint16_t color, uint16_t bg);
void draw_text(int x, int y, const char *s, uint16_t color, uint16_t bg);
void draw_char_fb(int x, int y, char c, uint8_t color, uint8_t bg);
void draw_text_fb(int x, int y, const char *s, uint8_t color, uint8_t bg);
void draw_char_fb_trans(int x, int y, char c, uint8_t color);
void draw_text_fb_trans(int x, int y, const char *s, uint8_t color);

#endif
