
#include "override_gpio.h"

#include "cge.h"

#include <stdlib.h>

struct override_gpio
{
    gpio_t target;
};

override_gpio_t override_gpio_create(gpio_t target)
{
    struct override_gpio *gpio = NULL;

    CGE_NULL(gpio = malloc(sizeof(struct override_gpio)));

    gpio->target = target;

    return gpio;

error:
    return NULL;
}

int override_gpio_set_direction(override_gpio_t gpio, enum gpio_direction direction)
{
    return gpio_set_direction(gpio->target, direction);
}

int override_gpio_read(override_gpio_t gpio)
{
    return gpio_read(gpio->target);
}

int override_gpio_write(override_gpio_t gpio, int value)
{
    return gpio_write(gpio->target, value);
}

void override_gpio_destroy(override_gpio_t gpio)
{
    free(gpio);
}
