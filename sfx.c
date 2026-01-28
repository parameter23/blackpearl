#include "sfx.h"

uint32_t SFX_TIMER_CLK_HZ = 0;

static uint32_t sfx_period = 0;

/* Laufender Effektzustand */
static SfxType   sfx_current      = SFX_NONE;
static uint32_t  sfx_time_left_ms = 0;
static uint32_t  sfx_param        = 0;   /* z.B. für Sweeps/Noise */

/* ---------------------------------------------------------
 * Hardware-Init
 * --------------------------------------------------------- */
void sfx_init(void)
{
    rcc_periph_clock_enable(SFX_TIMER_RCC);
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(SFX_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, SFX_GPIO_PIN);
    gpio_set_af(SFX_GPIO_PORT, SFX_GPIO_AF, SFX_GPIO_PIN);
    gpio_set_output_options(SFX_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, SFX_GPIO_PIN);

    timer_disable_counter(SFX_TIMER);

    timer_set_mode(SFX_TIMER,
                   TIM_CR1_CKD_CK_INT,
                   TIM_CR1_CMS_EDGE,
                   TIM_CR1_DIR_UP);

    uint32_t tim_clk = rcc_apb1_frequency * 2;
    uint32_t presc   = tim_clk / 1000000UL;
    if (presc == 0) presc = 1;

    timer_set_prescaler(SFX_TIMER, presc - 1);
    SFX_TIMER_CLK_HZ = tim_clk / presc;

    uint32_t period = SFX_TIMER_CLK_HZ / 1000UL;
    if (period < 2) period = 2;

    timer_set_period(SFX_TIMER, period);
    sfx_period = period;

    timer_set_oc_mode(SFX_TIMER, TIM_OC1, TIM_OCM_PWM1);
    timer_set_oc_value(SFX_TIMER, TIM_OC1, 0);
    timer_enable_oc_output(SFX_TIMER, TIM_OC1);

    timer_enable_counter(SFX_TIMER);
}

/* interne Helfer */
static void sfx_set_freq(uint32_t freq_hz)
{
    if (freq_hz == 0) {
        timer_set_oc_value(SFX_TIMER, TIM_OC1, 0);
        return;
    }

    uint32_t period = SFX_TIMER_CLK_HZ / freq_hz;
    if (period < 2) period = 2;
    if (period > 65535) period = 65535;

    timer_set_period(SFX_TIMER, period);
    sfx_period = period;
}

static void sfx_set_duty(uint32_t duty)
{
    if (duty > sfx_period) duty = sfx_period;
    timer_set_oc_value(SFX_TIMER, TIM_OC1, duty);
}

/* ---------------------------------------------------------
 * Non-blocking Update (1 ms Tick)
 * --------------------------------------------------------- */
void sfx_update_1ms(void)
{
    if (sfx_current == SFX_NONE)
        return;

    if (sfx_time_left_ms > 0) {
        sfx_time_left_ms--;
        if (sfx_time_left_ms == 0) {
            sfx_stop();
            return;
        }
    }

    /* Effekt-spezifische Laufzeitlogik */
    switch (sfx_current) {
    case SFX_LASER:
        /* einfacher Sweep nach oben */
        sfx_param += 5;  /* Schrittweite */
        if (sfx_param > 3000) sfx_param = 3000;
        sfx_set_freq(sfx_param);
        sfx_set_duty(sfx_period / 3);
        break;

    case SFX_EXPLOSION:
        /* grobe, pseudo-zufällige Frequenzsprünge */
        sfx_param = (sfx_param * 1103515245u + 12345u);
        {
            uint32_t f = 100 + (sfx_param >> 24) * 20; /* 100..~5200 Hz */
            sfx_set_freq(f);
            sfx_set_duty(sfx_period / 2);
        }
        break;

    case SFX_NOISE_SHORT:
        /* schnelles „Rauschen“ über Frequenzchaos */
        sfx_param = (sfx_param * 1664525u + 1013904223u);
        {
            uint32_t f = 200 + ((sfx_param >> 16) & 0xFF) * 15;
            sfx_set_freq(f);
            sfx_set_duty(sfx_period / 2);
        }
        break;

    default:
        break;
    }
}

/* ---------------------------------------------------------
 * Effekt starten
 * --------------------------------------------------------- */
void sfx_play(SfxType type)
{
    sfx_current      = type;
    sfx_time_left_ms = 0;
    sfx_param        = 1;

    switch (type) {
    case SFX_BEEP:
        sfx_set_freq(1000);
        sfx_set_duty(sfx_period / 2);
        sfx_time_left_ms = 200;   /* 200 ms */
        break;

    case SFX_LASER:
        sfx_param        = 200;   /* Startfreq */
        sfx_set_freq(sfx_param);
        sfx_set_duty(sfx_period / 3);
        sfx_time_left_ms = 300;
        break;

    case SFX_EXPLOSION:
        sfx_param        = 0x12345678;
        sfx_time_left_ms = 400;
        break;

    case SFX_NOISE_SHORT:
        sfx_param        = 0x87654321;
        sfx_time_left_ms = 150;
        break;

    default:
        sfx_stop();
        break;
    }
}

/* ---------------------------------------------------------
 * Effekt stoppen
 * --------------------------------------------------------- */
void sfx_stop(void)
{
    sfx_set_freq(0);
    sfx_current      = SFX_NONE;
    sfx_time_left_ms = 0;
    sfx_param        = 0;
}

#include <libopencm3/stm32/timer.h>

void sfx_play_freq(uint16_t freq)
{
    if (freq == 0) {
        sfx_stop();
        return;
    }

    /* Timer läuft mit 84 MHz (APB1 x2) → wir teilen auf 1 MHz runter */
    uint32_t timer_clk = 84000000;
    uint32_t prescaler = 83;       // 84 MHz / (83+1) = 1 MHz
    uint32_t period = (1000000 / freq) - 1;

    timer_set_prescaler(TIM3, prescaler);
    timer_set_period(TIM3, period);

    /* 50% Duty Cycle */
    timer_set_oc_value(TIM3, TIM_OC1, period / 2);

    timer_enable_counter(TIM3);
}
