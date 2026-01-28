#include "spi.h"
#include "framebuffer4.h"
#include "text.h"

#include <stdio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

#include "st7789.h"       // dein Display-Treiber
#include "joystick.h"     // Joystick-API
#include "diagnostic.h"   // joystick_draw_diag(), sfx_draw_diag()
#include "sfx.h"          // SFX-Engine


#include "clock.h"
#include "st7789.h"
#include "joystick.h"
#include "diagnostic.h"
#include "sfx.h"
#include "invaders.h"


// ------------------------------------------------------------
// MAIN
// ------------------------------------------------------------
int main(void)
{
    clock_setup();
    systick_setup();
    gpio_setup_for_st7789();
    spi_setup();
    st7789_init();
    fb4_init_palette();
    joystick_init();
    sfx_init();


invaders_init();

uint32_t last = millis();

while (1) {

    if (millis() - last >= 16) {   // ~60 FPS
        last = millis();

        invaders_update();
        invaders_render();
    }
}



}
