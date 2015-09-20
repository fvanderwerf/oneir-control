
#ifndef GPIO_SMBUS_H
#define GPIO_SMBUS_H

#include "gpio.h"

#include <stdint.h>

typedef struct gpio_smbus *gpio_smbus_t;

gpio_smbus_t gpio_smbus_create(gpio_t smbclk, gpio_t smbdat);

int gpio_smbus_write_word(gpio_smbus_t bus, uint8_t address, uint8_t command, uint16_t value);

void gpio_smbus_destroy(gpio_smbus_t bus);

#endif /* GPIO_SMBUS_H */
