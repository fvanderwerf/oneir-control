
#include "override_gpio.h"

#include "cge.h"

#include <stdlib.h>

struct override_gpio
{
    struct gpio gpio;

    gpio_t target;


    int override;

    enum gpio_direction target_direction;
    int target_output;
};

void override_gpio_setup_gpio(override_gpio_t gpio)
{
    gpio->gpio.destroy = (gpio_destroy_t) override_gpio_destroy;
    gpio->gpio.get_direction = (gpio_get_direction_t) override_gpio_get_direction;
    gpio->gpio.set_direction = (gpio_set_direction_t) override_gpio_set_direction;
    gpio->gpio.read = (gpio_read_t) override_gpio_read;
    gpio->gpio.write = (gpio_write_t) override_gpio_write;

    gpio->gpio.data = gpio;
}

override_gpio_t override_gpio_create(gpio_t target)
{
    struct override_gpio *gpio = NULL;

    CGE_NULL(gpio = malloc(sizeof(struct override_gpio)));

    gpio->target = target;
    gpio->override = 0;

    return gpio;

error:
    return NULL;
}

enum gpio_direction override_gpio_get_direction(override_gpio_t gpio)
{
    CGE_ERRNO(gpio->override, EPERM);

    return gpio_get_direction(gpio->target);
error:
    return -1;
}

int override_gpio_set_direction(override_gpio_t gpio, enum gpio_direction direction)
{
    CGE_ERRNO(gpio->override, EPERM);

    return gpio_set_direction(gpio->target, direction);
error:
    return -1;
}

int override_gpio_read(override_gpio_t gpio)
{
    CGE_ERRNO(gpio->override, EPERM);

    return gpio_read(gpio->target);

error:
    return -1;
}

int override_gpio_write(override_gpio_t gpio, int value)
{
    CGE_ERRNO(gpio->override, EPERM);

    return gpio_write(gpio->target, value);
error:
    return -1;
}

void override_gpio_destroy(override_gpio_t gpio)
{
    free(gpio);
}

int override_gpio_override(override_gpio_t gpio, int on)
{
    if (on && !gpio->override) {
        CGE_NEG(gpio->target_direction = gpio_get_direction(gpio->target));
        if (gpio->target_direction == GPIO_OUT)
            CGE_NEG(gpio->target_output = gpio_read(gpio->target));
    } else if (!on && gpio->override) {
        gpio_set_direction(gpio->target, gpio->target_direction);
        if (gpio->target_direction == GPIO_OUT)
            CGE_NEG(gpio_write(gpio->target, gpio->target_output));
    }

    gpio->override = on;

    return 0;

error:
    return -1;
}

int override_gpio_override_set_direction(override_gpio_t gpio, enum gpio_direction)
{
    CGE_ERRNO(!gpio->override, EPERM);
    
    return gpio_get_direction(gpio->target);
error:
    return -1;
}


int override_gpio_read(override_gpio_t gpio)
{
    CGE_ERRNO(!gpio->override, EPERM);

    return gpio_read(gpio->target);
error:
    return -1;
}

int override_gpio_write(override_gpio_t gpio, int value)
{
    CGE_ERRNO(!gpio->override, EPERM);

    return gpio_write(gpio->target, value);
error:
    return -1;
}
