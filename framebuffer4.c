#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/spi.h>
#include "framebuffer4.h"
#include "st7789.h"
#include "spi.h"

uint8_t framebuffer4[FB4_SIZE];
uint16_t fb4_palette[16];

static uint8_t fb4_linebuf[FB4_WIDTH * 2];

void fb4_clear(uint8_t color)
{
    // zwei Pixel pro Byte → beide Nibbles setzen
    uint8_t packed = (color << 4) | (color & 0x0F);

    for (int i = 0; i < FB4_SIZE; i++)
        framebuffer4[i] = packed;
}

void fb4_set_pixel(int x, int y, uint8_t color)
{
    if (x < 0 || y < 0 || x >= FB4_WIDTH || y >= FB4_HEIGHT)
        return;

    int index = (y * FB4_WIDTH + x) >> 1;  // /2
    int odd   = x & 1;

    if (odd)
        framebuffer4[index] = (framebuffer4[index] & 0xF0) | (color & 0x0F);
    else
        framebuffer4[index] = (framebuffer4[index] & 0x0F) | ((color & 0x0F) << 4);
}

uint8_t fb4_get_pixel(int x, int y)
{
    if (x < 0 || y < 0 || x >= FB4_WIDTH || y >= FB4_HEIGHT)
        return 0;

    int index = (y * FB4_WIDTH + x) >> 1;
    int odd   = x & 1;

    uint8_t b = framebuffer4[index];
    return odd ? (b & 0x0F) : (b >> 4);
}

void fb4_flush(void)
{
    st7789_set_window(0, 0, FB4_WIDTH - 1, FB4_HEIGHT - 1);

    CS_LOW();
    DC_DATA();

    for (int i = 0; i < FB4_SIZE; i++) {
        uint8_t packed = framebuffer4[i];

        uint8_t hi = packed >> 4;
        uint8_t lo = packed & 0x0F;

        uint16_t c1 = fb4_palette[hi];
        uint16_t c2 = fb4_palette[lo];

        spi_write8(c1 >> 8);
        spi_write8(c1 & 0xFF);

        spi_write8(c2 >> 8);
        spi_write8(c2 & 0xFF);
    }

    CS_HIGH();
}

static void fb4_dma_init_once(void)
{
    static int initialized = 0;
    if (initialized) return;
    initialized = 1;

    rcc_periph_clock_enable(RCC_DMA2);

    dma_stream_reset(DMA2, DMA_STREAM3);

    dma_channel_select(DMA2, DMA_STREAM3, DMA_SxCR_CHSEL_3); // SPI1_TX

    dma_set_priority(DMA2, DMA_STREAM3, DMA_SxCR_PL_HIGH);
    dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);

    dma_enable_memory_increment_mode(DMA2, DMA_STREAM3);
    dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM3);

    dma_set_peripheral_size(DMA2, DMA_STREAM3, DMA_SxCR_PSIZE_8BIT);
    dma_set_memory_size(DMA2, DMA_STREAM3, DMA_SxCR_MSIZE_8BIT);

    dma_set_peripheral_address(DMA2, DMA_STREAM3, (uint32_t)&SPI_DR(SPI1));
}

static void fb4_dma_send_line(uint8_t *buf, uint16_t count)
{
    dma_stream_reset(DMA2, DMA_STREAM3);

    dma_channel_select(DMA2, DMA_STREAM3, DMA_SxCR_CHSEL_3);
    dma_set_priority(DMA2, DMA_STREAM3, DMA_SxCR_PL_HIGH);
    dma_set_transfer_mode(DMA2, DMA_STREAM3, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);

    dma_enable_memory_increment_mode(DMA2, DMA_STREAM3);
    dma_disable_peripheral_increment_mode(DMA2, DMA_STREAM3);

    dma_set_peripheral_size(DMA2, DMA_STREAM3, DMA_SxCR_PSIZE_8BIT);
    dma_set_memory_size(DMA2, DMA_STREAM3, DMA_SxCR_MSIZE_8BIT);

    dma_set_peripheral_address(DMA2, DMA_STREAM3, (uint32_t)&SPI_DR(SPI1));
    dma_set_memory_address(DMA2, DMA_STREAM3, (uint32_t)buf);
    dma_set_number_of_data(DMA2, DMA_STREAM3, count);

    dma_clear_interrupt_flags(DMA2, DMA_STREAM3, DMA_TCIF);

    spi_enable_tx_dma(SPI1);
    dma_enable_stream(DMA2, DMA_STREAM3);

    while (!dma_get_interrupt_flag(DMA2, DMA_STREAM3, DMA_TCIF))
        ;

    dma_disable_stream(DMA2, DMA_STREAM3);
    spi_disable_tx_dma(SPI1);
}

void fb4_flush_dma(void)
{
    fb4_dma_init_once();

    spi_disable(SPI1);
    spi_set_dff_8bit(SPI1);
    spi_enable(SPI1);

    st7789_set_window(0, 0, FB4_WIDTH - 1, FB4_HEIGHT - 1);

    CS_LOW();
    DC_DATA();

    for (int y = 0; y < FB4_HEIGHT; y++) {
        int fb_index = (y * FB4_WIDTH) >> 1;
        int out = 0;

        for (int x = 0; x < FB4_WIDTH; x += 2) {
            uint8_t packed = framebuffer4[fb_index++];

            uint8_t hi = packed >> 4;
            uint8_t lo = packed & 0x0F;

            uint16_t c1 = fb4_palette[hi];
            uint16_t c2 = fb4_palette[lo];

            fb4_linebuf[out++] = c1 >> 8;
            fb4_linebuf[out++] = c1 & 0xFF;
            fb4_linebuf[out++] = c2 >> 8;
            fb4_linebuf[out++] = c2 & 0xFF;
        }

        fb4_dma_send_line(fb4_linebuf, FB4_WIDTH * 2);
    }

    CS_HIGH();
}

void fb4_init_palette(void)
{
  memcpy(fb4_palette, fb4_default_palette, sizeof(fb4_palette));
}

void fb4_fill_rect(int x, int y, int w, int h, uint8_t color)
{
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            fb4_set_pixel(xx, yy, color);
        }
    }
}
