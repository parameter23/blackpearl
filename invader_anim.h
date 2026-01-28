#pragma once
#include "sprite.h"

void invader_anim_init(void);
void invader_anim_update(void);

const Sprite4 *invader_anim_get(uint8_t type, uint8_t frame);

extern uint8_t anim_frame;
