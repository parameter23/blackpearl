#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/adc.h>
#include "joystick.h"

// interne ADC-Werte
static uint16_t js_adc_x = 2048;
static uint16_t js_adc_y = 2048;

// Auto-Repeat Timer
static uint32_t repeat_timer = 0;
static uint8_t last_raw = 0;

void joystick_init(void)
{
    // --- Digitale Pins (GPIOB) ---
    rcc_periph_clock_enable(RCC_GPIOB);

    gpio_mode_setup(JS_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP,
                    JS_UP_PIN | JS_DOWN_PIN | JS_LEFT_PIN |
                    JS_RIGHT_PIN | JS_BTN_PIN);

    // --- Analoge Pins (PA0, PA1) ---
    rcc_periph_clock_enable(RCC_GPIOA);

    gpio_mode_setup(JS_ADC_X_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE,
                    JS_ADC_X_PIN | JS_ADC_Y_PIN);

    // --- ADC Setup ---
    rcc_periph_clock_enable(RCC_ADC1);

    adc_power_off(ADC1);
    adc_disable_scan_mode(ADC1);
    adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_144CYC);
    adc_power_on(ADC1);
}

// Hilfsfunktion: ADC lesen
static uint16_t adc_read_channel(uint8_t ch)
{
    adc_set_regular_sequence(ADC1, 1, &ch);
    adc_start_conversion_regular(ADC1);
    while (!adc_eoc(ADC1));
    return adc_read_regular(ADC1);
}

JoystickState joystick_update(void)
{
    JoystickState js = {0};

    // --- Digitale Eingänge ---
    uint8_t raw = 0;

    if (!gpio_get(JS_PORT, JS_UP_PIN))    raw |= 0x01;
    if (!gpio_get(JS_PORT, JS_DOWN_PIN))  raw |= 0x02;
    if (!gpio_get(JS_PORT, JS_LEFT_PIN))  raw |= 0x04;
    if (!gpio_get(JS_PORT, JS_RIGHT_PIN)) raw |= 0x08;
    if (!gpio_get(JS_PORT, JS_BTN_PIN))   raw |= 0x10;

    js.raw = raw;

    // --- Flanken ---
    js.pressed  = (raw & ~last_raw);
    js.released = (~raw & last_raw);

    // --- Auto-Repeat ---
    if (raw) {
        repeat_timer++;
        if (repeat_timer > 10) {
            js.repeat = raw;
            repeat_timer = 0;
        }
    } else {
        repeat_timer = 0;
    }

    last_raw = raw;

    // --- Analoge Werte ---
    js_adc_x = adc_read_channel(0); // PA0
    js_adc_y = adc_read_channel(1); // PA1

    return js;
}

// Getter
uint16_t joystick_get_adc_x(void) { return js_adc_x; }
uint16_t joystick_get_adc_y(void) { return js_adc_y; }
