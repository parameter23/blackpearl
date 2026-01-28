#ifndef CLOCK_H
#define CLOCK_H

#include <stdint.h>

void clock_setup(void);
void systick_setup(void);
void sys_tick_handler(void);

uint32_t millis(void);
void delay_ms(uint32_t ms);

void gpio_setup_for_st7789(void);

#endif
