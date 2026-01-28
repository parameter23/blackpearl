#include "sprite.h"
#include "framebuffer4.h"

#include "sprite.h"

static const uint8_t sprite_test_data[] = {
    0x11,0x11,0x11,0x11,
    0x12,0x22,0x22,0x21,
    0x12,0x22,0x22,0x21,
    0x12,0x22,0x22,0x21,
    0x12,0x22,0x22,0x21,
    0x12,0x22,0x22,0x21,
    0x12,0x22,0x22,0x21,
    0x11,0x11,0x11,0x11
};

const Sprite4 sprite_test = {
    .width = 8,
    .height = 8,
    .data = sprite_test_data
};

void sprite4_draw(int x, int y, const Sprite4 *spr)
{
    int idx = 0;

    for (int py = 0; py < spr->height; py++) {
        for (int px = 0; px < spr->width; px++) {

            uint8_t packed = spr->data[idx >> 1];
            uint8_t color = (idx & 1) ? (packed & 0x0F) : (packed >> 4);

            fb4_set_pixel(x + px, y + py, color);

            idx++;
        }
    }
}

void sprite4_draw_trans(int x, int y, const Sprite4 *spr, uint8_t transparent)
{
    int idx = 0;

    for (int py = 0; py < spr->height; py++) {
        for (int px = 0; px < spr->width; px++) {

            uint8_t packed = spr->data[idx >> 1];
            uint8_t color = (idx & 1) ? (packed & 0x0F) : (packed >> 4);

            if (color != transparent)
                fb4_set_pixel(x + px, y + py, color);

            idx++;
        }
    }
}
