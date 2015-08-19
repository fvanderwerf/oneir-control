
#include "oneir_bus.h"

#include "cge.h"

#include <stdlib.h>


struct oneir_bus {

    override_gpio_t sclk;
    override_gpio_t miso;
    override_gpio_t mosi;
};

oneir_bus_t oneir_bus_create(override_gpio_t sclk, override_gpio_t miso, override_gpio_t mosi)
{
    struct oneir_bus *bus;

    CGE_NULL(bus = malloc(sizeof(struct oneir_bus)));

    bus->sclk = sclk;
    bus->miso = miso;
    bus->mosi = mosi;

    oneir_bus_select(bus, ONEIR_I2C);

    return bus;
error:

    return NULL;
}

int oneir_bus_select(oneir_bus_t bus, enum oneir_bus_type type)
{
    if (type == ONEIR_I2C) {
        /* decouple SPI pins by setting them to input */
        CGE_NEG(override_gpio_override(bus->sclk, 1));
        CGE_NEG(override_gpio_override(bus->miso, 1));
        CGE_NEG(override_gpio_override(bus->mosi, 1));

        CGE_NEG(override_gpio_override_set_direction(bus->sclk, GPIO_IN));
        CGE_NEG(override_gpio_override_set_direction(bus->miso, GPIO_IN));
        CGE_NEG(override_gpio_override_set_direction(bus->mosi, GPIO_IN));
    } else {
        CGE_NEG(override_gpio_override(bus->sclk, 0));
        CGE_NEG(override_gpio_override(bus->miso, 0));
        CGE_NEG(override_gpio_override(bus->mosi, 0));
    }

    return 0;
error:
    return -1;
}

void oneir_bus_destroy(oneir_bus_t bus)
{
    free(bus);
}
