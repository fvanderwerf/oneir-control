
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "sysfs_gpio.h"
#include "gpio_spi.h"
#include "avr.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <stdint.h>


sysfs_gpio_t sclk = NULL, miso = NULL, mosi = NULL, reset = NULL;
gpio_spi_t spi = NULL;
avr_t avr = NULL;


void cleanup();


int setup()
{
    sclk = sysfs_gpio_create(11);
    miso = sysfs_gpio_create(9);
    mosi = sysfs_gpio_create(10);
    reset = sysfs_gpio_create(25);

    if (sclk == NULL || miso == NULL || mosi == NULL || reset == NULL)
        goto error;

    spi = gpio_spi_create(
            sysfs_gpio_to_gpio(sclk),
            sysfs_gpio_to_gpio(mosi),
            sysfs_gpio_to_gpio(miso));
    if (spi == NULL)
        goto error;

    avr = avr_create(spi, sysfs_gpio_to_gpio(reset));
    if (avr == NULL)
        goto error;

    return 0;

error:
    cleanup();

    return -1;
}

void cleanup()
{
    if (avr != NULL)
        avr_destroy(avr);

    if (spi != NULL)
        gpio_spi_destroy(spi);

    if (reset != NULL)
        sysfs_gpio_destroy(reset);

    if (sclk != NULL)
        sysfs_gpio_destroy(sclk);

    if (miso != NULL)
        sysfs_gpio_destroy(miso);

    if (mosi != NULL)
        sysfs_gpio_destroy(mosi);
}



int main(int argc, char *argv)
{
    int exitcode = 0;

    if (setup() != 0) {
        printf("setup failed\n");
        exit(1);
    }

    if (avr_reset(avr) != 0)
    {
        printf("give reset puls\n");
        exit(1);
    }

    printf("program enable\n");
    if (avr_program_enable(avr)) {
        perror("avr program enable");
        printf("program enable failed\n");
    }

#if 0
    switch (avr_read_signature(avr)) {
        case ATTINY_UNKNOWN:
            printf("unknown attiny detected\n");
            break;
        case ATTINY25:
            printf("ATtiny25 detected\n");
            break;
        case ATTINY45:
            printf("ATtiny45 detected\n");
            break;
        case ATTINY85:
            printf("ATtiny85 detected\n");
            break;
        default:
            perror("signature");
            exit(1);
            break;
    }
#endif

    avr_unreset(avr);
    cleanup();

    return 0;
}
