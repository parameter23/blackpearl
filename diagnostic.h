#ifndef DIAGNOSTIC_H
#define DIAGNOSTIC_H

#include <stdint.h>
#include "joystick.h"

void sfx_draw_diag(int x, int y);
void joystick_draw_diag(int x, int y, JoystickState *js,
                        uint16_t ax, uint16_t ay, uint8_t dir);

#endif
