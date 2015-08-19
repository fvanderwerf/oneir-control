
#ifndef GPIO_H
#define GPIO_H

enum gpio_direction
{
    GPIO_IN,
    GPIO_OUT
};

typedef struct gpio *gpio_t;

typedef void (*gpio_destroy_t)(void *data);
typedef enum gpio_direction (*gpio_get_direction_t)(void *data);
typedef int (*gpio_set_direction_t)(void *data, enum gpio_direction direction);
typedef int  (*gpio_read_t)(void *data);
typedef int (*gpio_write_t)(void *data, int value);

struct gpio {
    gpio_destroy_t destroy;
    gpio_get_direction_t get_direction;
    gpio_set_direction_t set_direction;
    gpio_read_t read;
    gpio_write_t write;

    void *data;
};


static inline void gpio_destroy(gpio_t gpio)
{
    gpio->destroy(gpio->data);
}

static inline enum gpio_direction gpio_get_direction(gpio_t gpio)
{
    return gpio->get_direction(gpio->data);
}

static inline int gpio_set_direction(gpio_t gpio, enum gpio_direction direction)
{
    return gpio->set_direction(gpio->data, direction);
}

static inline int gpio_read(gpio_t gpio)
{
    return gpio->read(gpio->data);
}

static inline int gpio_write(gpio_t gpio, int value)
{
    return gpio->write(gpio->data, value);
}

#endif /* GPIO_H */

