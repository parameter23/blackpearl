#ifndef SPRITE_H
#define SPRITE_H

#include <stdint.h>

typedef struct {
    uint8_t width;
    uint8_t height;
    const uint8_t *data;   // 4-bit packed
} Sprite4;

extern const Sprite4 sprite_test;

void sprite4_draw(int x, int y, const Sprite4 *spr);
void sprite4_draw_trans(int x, int y, const Sprite4 *spr, uint8_t transparent);

#endif
