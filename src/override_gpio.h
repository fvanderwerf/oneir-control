
#ifndef OVERRIDE_GPIO_H
#define OVERRIDE_GPIO_H

#include "gpio.h" 

typedef struct override_gpio *override_gpio_t;

override_gpio_t override_gpio_create();

void override_gpio_destroy(override_gpio_t gpio);

enum gpio_direction override_gpio_get_direction(override_gpio_t gpio);

int override_gpio_set_direction(override_gpio_t gpio, enum gpio_direction direction);

int override_gpio_read(override_gpio_t gpio);

int override_gpio_write(override_gpio_t gpio, int value);

int override_gpio_override(override_gpio_t gpio, int on);

enum gpio_direction override_gpio_override_get_direction(override_gpio_t gpio);

int override_gpio_override_set_direction(override_gpio_t gpio, enum gpio_direction direction);

int override_gpio_override_read(override_gpio_t gpio);

int override_gpio_override_write(override_gpio_t gpio, int value);

#endif /* OVERRIDE_GPIO_H */
