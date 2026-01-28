#pragma once
#include <stdint.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

/* Hardware-Zuordnung */
#define SFX_TIMER       TIM3
#define SFX_TIMER_RCC   RCC_TIM3
#define SFX_GPIO_PORT   GPIOA
#define SFX_GPIO_PIN    GPIO6
#define SFX_GPIO_AF     GPIO_AF2

extern uint32_t SFX_TIMER_CLK_HZ;

/* SFX-Typen */
typedef enum {
    SFX_NONE = 0,
    SFX_BEEP,
    SFX_LASER,
    SFX_EXPLOSION,
    SFX_NOISE_SHORT,
} SfxType;

/* API */
void sfx_init(void);
void sfx_update_1ms(void);          /* im SysTick aufrufen */
void sfx_play(SfxType type);        /* Effekt triggern */
void sfx_stop(void);                /* hart stoppen */
void sfx_play_freq(uint16_t freq);
