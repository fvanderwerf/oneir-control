
#include "avr.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct avr {
    gpio_spi_t spi;
    gpio_t reset;
};


avr_t avr_create(gpio_spi_t spi, gpio_t reset)
{

    struct avr *avr = malloc(sizeof(struct avr));
    if (avr == NULL)
        goto error;

    if (gpio_set_direction(reset, GPIO_OUT) != 0)
        goto error;

    avr->spi = spi;
    avr->reset = reset;

    return avr;

error:
    if (avr != NULL)
        free(avr);

    return NULL;
}

int avr_reset(avr_t avr)
{
    if (sysfs_gpio_write(avr->reset, 0) != 0)
        goto error;

    usleep(1000);

    if (sysfs_gpio_write(avr->reset, 1) != 0)
        goto error;
        
    usleep(1000);

    if (sysfs_gpio_write(avr->reset, 0) != 0)
        goto error;

    usleep(20000);

    return 0;

error:
    return -1;
}

int avr_unreset(avr_t avr)
{
    if (sysfs_gpio_write(avr->reset, 1) != 0)
        goto error;

    return 0;

error:
    return -1;
}


int avr_program_enable(avr_t avr)
{
    char *tx = "\xac\x53\x00\x00";
    char rx[4];

    if (gpio_spi_transfer(avr->spi, tx, rx, 4) != 0)
        goto error;
    
    if (rx[2] != '\x53') {
        errno = EPROTO;
        goto error;
    }

    return 0;
error:
    return -1;
}


static enum attiny avr_parse_signature(const char *signature)
{
    if (memcmp(signature, "\x1e\x91\x08", 3) == 0)
        return ATTINY25;
    else if (memcmp(signature, "\x1e\x92\x06", 3) == 0)
        return ATTINY45;
    else if (memcmp(signature, "\x1e\x93\x0b", 3) == 0)
        return ATTINY85;
    else
        return ATTINY_UNKNOWN;
}


enum attiny avr_read_signature(avr_t avr)
{
    const char *tx;
    char rx[4];
    char signature[3];

    tx = "\x30\x00\x00\x00";
    if (gpio_spi_transfer(avr->spi, tx, rx, 4) != 0)
        goto error;

    signature[0] = rx[3];

    tx = "\x30\x00\x01\x00";
    if (gpio_spi_transfer(avr->spi, tx, rx, 4) != 0)
        goto error;

    signature[1] = rx[3];

    tx = "\x30\x00\x02\x00";
    if (gpio_spi_transfer(avr->spi, tx, rx, 4) != 0)
        goto error;
    signature[2] = rx[3];

    return avr_parse_signature(signature);

error:
    return -1;
}

void avr_destroy(avr_t avr)
{
    free(avr);
}
