
#ifndef SYSFS_GPIO_H
#define SYSFS_GPIO_H

#include "gpio.h"


typedef struct sysfs_gpio *sysfs_gpio_t;

sysfs_gpio_t sysfs_gpio_create(int gpio_num);

gpio_t sysfs_gpio_to_gpio(sysfs_gpio_t gpio);

enum gpio_direction sysfs_gpio_get_direction(sysfs_gpio_t gpio);

int sysfs_gpio_set_direction(sysfs_gpio_t gpio, enum gpio_direction direction);

int sysfs_gpio_read(sysfs_gpio_t gpio);

int sysfs_gpio_write(sysfs_gpio_t gpio, int value);

void sysfs_gpio_destroy(sysfs_gpio_t gpio);


#endif /* SYSFS_GPIO_H */
