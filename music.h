#pragma once
#include <stdint.h>

void music_init(void);
void music_update_1ms(void);
void music_play(void);
void music_stop(void);

extern uint8_t music_enabled;
