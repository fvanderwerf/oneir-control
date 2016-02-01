
#include "gpio_smbus.h"
#include "cge.h"
#include <stdlib.h>

struct gpio_smbus {
    gpio_t smbclk;
    gpio_t smbdat;
};

gpio_smbus_t gpio_smbus_create(gpio_t smbclk, gpio_t smbdat)
{
    struct gpio_smbus *bus = NULL;
   
    CGE_NULL(bus = malloc(sizeof(struct gpio_smbus)));

    bus->smbclk = smbclk;
    bus->smbdat = smbdat;

    CGE_NEG(gpio_set_direction(bus->smbclk, GPIO_IN));
    CGE_NEG(gpio_set_direction(bus->smbdat, GPIO_IN));

    return bus;

error:
    if (bus)
        free(bus);

    return NULL;
}

static int gpio_smbus_pull_down(gpio_t gpio)
{
    CGE_NEG(gpio_set_direction(gpio, GPIO_OUT));
    CGE_NEG(gpio_write(gpio, 0));

    return 0;

error:
    return -1;
}


static int gpio_smbus_release(gpio_t gpio)
{
    CGE_NEG(gpio_set_direction(gpio, GPIO_IN));

    return 0;

error:
    return -1;
}

static int gpio_smbus_wait_release(gpio_t gpio)
{
    int readvalue;

    do {
        CGE_NEG(readvalue = gpio_read(gpio));
    } while (readvalue == 0);
    
    return 0;

error:
    return -1;
}

static int gpio_smbus_generate_start_condition(gpio_smbus_t bus)
{
    CGE_NEG(gpio_smbus_pull_down(bus->smbdat));
    CGE_NEG(gpio_smbus_pull_down(bus->smbclk));

    return 0;

error:
    return -1;
}

static int gpio_smbus_generate_stop_condition(gpio_smbus_t bus)
{
    /* release clock */
    CGE_NEG(gpio_smbus_release(bus->smbclk));

    /* wait for client to release clock as well */
    CGE_NEG(gpio_smbus_wait_release(bus->smbclk));

    /* release smbdat */
    CGE_NEG(gpio_smbus_release(bus->smbdat));

    return 0;

error:
    return -1;
}



static int gpio_smbus_send_byte(gpio_smbus_t bus, uint8_t byte)
{
    int i;
    int ack;
    /* send address and read/write bit */
    for (i = 0; i < 8; i++) {
        /* set smbdat */
        if ((byte << i) & 0x80)
            CGE_NEG(gpio_smbus_release(bus->smbdat));
        else
            CGE_NEG(gpio_smbus_pull_down(bus->smbdat));

        /* release clock */
        CGE_NEG(gpio_smbus_release(bus->smbclk));

        /* wait for slave to release clock as well */
        CGE_NEG(gpio_smbus_wait_release(bus->smbclk));

        /* pull down clock */
        CGE_NEG(gpio_smbus_pull_down(bus->smbclk));
    }
    /* now start reading ack */

    /* release smbdat */
    CGE_NEG(gpio_smbus_release(bus->smbdat));

    /* release clock */
    CGE_NEG(gpio_smbus_release(bus->smbclk));

    /* wait for client to release clock as well */
    CGE_NEG(gpio_smbus_wait_release(bus->smbclk));


    /* read ack */
    CGE_NEG(ack = gpio_read(bus->smbdat));

    /* pull down clock */
    CGE_NEG(gpio_smbus_pull_down(bus->smbclk));

    /* pull down smbdat */
    CGE_NEG(gpio_smbus_pull_down(bus->smbclk));


    return ack;

error:
    return -1;
}


int gpio_smbus_write_word(gpio_smbus_t bus, uint8_t address, uint8_t command, uint16_t value)
{
    /* add read/write bit to address */
    address <<= 1; 

    CGE_NEG(gpio_smbus_generate_start_condition(bus));

    CGE_NEG(gpio_smbus_send_byte(bus, address));

    CGE_NEG(gpio_smbus_send_byte(bus, command));

    CGE_NEG(gpio_smbus_send_byte(bus, value & 0xFF));

    CGE_NEG(gpio_smbus_send_byte(bus, (value >> 8) & 0xFF));
    
    CGE_NEG(gpio_smbus_generate_stop_condition(bus));

    return 0;

error:
    return -1;
}

int gpio_smbus_write_buffer(gpio_smbus_t bus, uint8_t address, const char *buf, size_t buflen)
{
    int i;

    address <<= 1;

    CGE_NEG(gpio_smbus_generate_start_condition(bus));

    CGE_NEG(gpio_smbus_send_byte(bus, address));

    for (i = 0; i < buflen; i++) {
        CGE_NEG(gpio_smbus_send_byte(bus, buf[i]));
    }

    CGE_NEG(gpio_smbus_generate_stop_condition(bus));

    return 0;

error:
    return -1;
}

void gpio_smbus_destroy(gpio_smbus_t bus)
{
    gpio_set_direction(bus->smbclk, GPIO_IN);
    gpio_set_direction(bus->smbdat, GPIO_IN);

    free(bus);
}
