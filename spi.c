#include "spi.h"
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>

void spi_setup(void)
{
    rcc_periph_clock_enable(RCC_SPI1);

    spi_init_master(
        SPI1,
        SPI_CR1_BAUDRATE_FPCLK_DIV_2,
        SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
        SPI_CR1_CPHA_CLK_TRANSITION_2,
        SPI_CR1_DFF_8BIT,
        SPI_CR1_MSBFIRST
    );

    spi_enable(SPI1);
}

void spi_write8(uint8_t data)
{
    spi_send(SPI1, data);
    spi_read(SPI1);
}
