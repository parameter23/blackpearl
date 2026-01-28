#include "diagnostic.h"
#include "framebuffer4.h"
#include "text.h"
#include "joystick.h"
#include <stdio.h>

void joystick_draw_diag(int x, int y, JoystickState *js,
                        uint16_t ax, uint16_t ay, uint8_t dir)
{
    // Hintergrundkasten
    fb4_fill_rect(x, y, 200, 120, 2); // Farbe 2 = dunkelgrau

    int cy = y + 4;

    draw_text_fb_trans(x + 4, cy, "JOYSTICK DIAG", 1);
    cy += 12;

    // RAW
    char buf[32];
    snprintf(buf, sizeof(buf), "RAW:   %02X", js->raw);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 10;

    // PRESSED
    snprintf(buf, sizeof(buf), "PRESS: %02X", js->pressed);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 10;

    // REPEAT
    snprintf(buf, sizeof(buf), "REPT:  %02X", js->repeat);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 10;

    // RELEASED
    snprintf(buf, sizeof(buf), "REL:   %02X", js->released);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 14;

    // ANALOG
    draw_text_fb_trans(x + 4, cy, "ANALOG:", 1);
    cy += 10;

    snprintf(buf, sizeof(buf), "X: %4u", ax);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 10;

    snprintf(buf, sizeof(buf), "Y: %4u", ay);
    draw_text_fb_trans(x + 4, cy, buf, 1);
    cy += 10;

    // Richtung
    const char *d = "NONE";
    if (dir & JS_LEFT)  d = "LEFT";
    if (dir & JS_RIGHT) d = "RIGHT";
    if (dir & JS_UP)    d = "UP";
    if (dir & JS_DOWN)  d = "DOWN";

    snprintf(buf, sizeof(buf), "DIR: %s", d);
    draw_text_fb_trans(x + 4, cy, buf, 1);
}
