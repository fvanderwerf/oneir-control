
#ifndef GPIO_SPI_H
#define GPIO_SPI_H

#include "gpio.h"
#include <stddef.h>

typedef struct gpio_spi *gpio_spi_t;

gpio_spi_t gpio_spi_create(gpio_t sclk, gpio_t mosi, gpio_t miso);

int gpio_spi_transfer(gpio_spi_t spi, const void *out, void *in, size_t len);

void gpio_spi_destroy(gpio_spi_t spi);

#endif /* GPIO_SPI_H */

