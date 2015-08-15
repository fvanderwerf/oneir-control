
#include "gpio_spi.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define GET_BIT(data, byte, bit)                ((((uint8_t *) data)[byte]) >> (7 - bit)) & 1
#define SET_BIT(data, byte, bit, value)                     \
do {                                                        \
    if (value) {                                            \
        ((uint8_t *) data)[byte] |=  1 << (7 - bit);        \
    } else {                                                \
        ((uint8_t *) data)[byte] &=  ~(1 << (7 - bit));     \
    }                                                       \
} while(0)


struct gpio_spi {
    gpio_t sclk;
    gpio_t mosi;
    gpio_t miso;
};


gpio_spi_t gpio_spi_create(gpio_t sclk, gpio_t mosi, gpio_t miso)
{
    gpio_spi_t spi = malloc(sizeof(struct gpio_spi));
    if (spi == NULL)
        goto error;

    spi->sclk = sclk;
    spi->mosi = mosi;
    spi->miso = miso;

    if (gpio_set_direction(sclk, GPIO_OUT) != 0)
        goto error;

    if (gpio_set_direction(mosi, GPIO_OUT) != 0)
        goto error;

    if (gpio_set_direction(miso, GPIO_IN) != 0)
        goto error;

    gpio_write(sclk, 0);

    return spi;

error:
    if (spi != NULL)
        free(spi);

    return NULL;
}


int gpio_spi_transfer(gpio_spi_t spi, const void *out, void *in, size_t len)
{
    int byte, bit;
    int bitvalue;
      
    for (byte = 0; byte < len; byte++) {
        for (bit = 0; bit < 8; bit++)  {

            if (gpio_write(spi->mosi, GET_BIT(out, byte, bit)) == -1)
                goto error;

            usleep(100);
            if (gpio_write(spi->sclk, 1) == -1)
                goto error;

            usleep(100);

            if ((bitvalue = gpio_read(spi->miso)) == -1)
                goto error;

            SET_BIT(in, byte, bit, bitvalue);

            if (gpio_write(spi->sclk, 0) == -1)
                goto error;
        }
    }

    return 0;

error:
    return -1;
}


void gpio_spi_destroy(gpio_spi_t spi)
{
    free(spi);
}
