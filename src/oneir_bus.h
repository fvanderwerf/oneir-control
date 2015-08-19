
#ifndef ONEIR_BUS_H
#define ONEIR_BUS_H

#include "override_gpio.h"

enum oneir_bus_type {
    ONEIR_SPI,
    ONEIR_I2C
};

typedef struct oneir_bus *oneir_bus_t;

oneir_bus_t oneir_bus_create(override_gpio_t sclk, override_gpio_t miso, override_gpio_t mosi);

int oneir_bus_select(oneir_bus_t bus, enum oneir_bus_type type);

void oneir_bus_destroy(oneir_bus_t bus);


#endif /* ONEIR_BUS_H */
