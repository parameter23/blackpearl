#include "title_screen.h"
#include "framebuffer4.h"
#include "text.h"
#include "joystick.h"
#include "sfx.h"

int title_screen_active = 1;

void title_screen_init(void)
{
    title_screen_active = 1;
}

void title_screen_update(void)
{
    if (!title_screen_active)
        return;

    JoystickState js = joystick_update();

    if (js.pressed & JS_BTN) {
        sfx_play(SFX_BEEP);
        title_screen_active = 0;
    }
}

void title_screen_render(void)
{
    if (!title_screen_active)
        return;

    fb4_clear(0);

    draw_text_fb_trans(40, 60, "STM32 INVADERS", 1);
    draw_text_fb_trans(40, 90, "PRESS BUTTON", 1);
    draw_text_fb_trans(40, 110, "TO START", 1);

    fb4_flush_dma();
}
