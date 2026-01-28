#include "clock.h"
#include "sfx.h"
#include "music.h"
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/systick.h>

static volatile uint32_t ms_ticks = 0;

void sys_tick_handler(void)
{
    ms_ticks++;
    sfx_update_1ms();
    music_update_1ms();
}

uint32_t millis(void)
{
    return ms_ticks;
}

void delay_ms(uint32_t ms)
{
    uint32_t start = millis();
    while ((millis() - start) < ms);
}

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>

void clock_setup(void)
{
    /* 1. HSE einschalten */
    rcc_osc_on(RCC_HSE);
    rcc_wait_for_osc_ready(RCC_HSE);

    /* 2. Prescaler setzen:
       AHB  = /1
       APB1 = /2
       APB2 = /1
    */
    RCC_CFGR &= ~(RCC_CFGR_HPRE_MASK |
                  RCC_CFGR_PPRE1_MASK |
                  RCC_CFGR_PPRE2_MASK);

    RCC_CFGR |= RCC_CFGR_HPRE_NODIV;   // AHB = SYSCLK
    RCC_CFGR |= RCC_CFGR_PPRE_DIV2;    // APB1 = AHB/2
    RCC_CFGR |= RCC_CFGR_PPRE_NODIV;   // APB2 = AHB

    /* 3. PLL konfigurieren: 25MHz → 100MHz */
    RCC_PLLCFGR =
          RCC_PLLCFGR_PLLSRC          /* Bit gesetzt = HSE */
        | (25  << RCC_PLLCFGR_PLLM_SHIFT)
        | (200 << RCC_PLLCFGR_PLLN_SHIFT)
        | (0   << RCC_PLLCFGR_PLLP_SHIFT)   /* PLLP = 2 */
        | (4   << RCC_PLLCFGR_PLLQ_SHIFT);

    /* 4. PLL einschalten */
    rcc_osc_on(RCC_PLL);
    rcc_wait_for_osc_ready(RCC_PLL);

    /* 5. Flash Waitstates */
    flash_set_ws(FLASH_ACR_LATENCY_3WS);

    /* 6. PLL als Systemclock */
    rcc_set_sysclk_source(RCC_CFGR_SW_PLL);

    /* 7. Frequenzen korrekt setzen */
    rcc_ahb_frequency  = 100000000;
    rcc_apb1_frequency = 50000000;
    rcc_apb2_frequency = 100000000;
}

void systick_setup(void)
{
    systick_set_reload(100000 - 1);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_clear();
    systick_counter_enable();
    systick_interrupt_enable();
}

void gpio_setup_for_st7789(void)
{
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO5 | GPIO7);
    gpio_set_af(GPIOA, GPIO_AF5, GPIO5 | GPIO7);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO5 | GPIO7);

    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO4);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO4);
    gpio_set(GPIOA, GPIO4);

    gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO2 | GPIO3);
    gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO2 | GPIO3);
}
