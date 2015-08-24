
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "sysfs_gpio.h"
#include "gpio_spi.h"
#include "avr.h"
#include "intel_hex.h"
#include "oneir_mcu.h"
#include "cge.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <stdint.h>


sysfs_gpio_t sclk = NULL, miso = NULL, mosi = NULL, reset = NULL;
gpio_spi_t spi = NULL;
avr_t avr = NULL;
oneir_mcu_t mcu = NULL;


void cleanup();


int setup(const char *filename)
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

    CGE_NULL(mcu = oneir_mcu_create(avr));

    return 0;

error:
    cleanup();

    return -1;
}

void cleanup()
{
    if (mcu != NULL)
        oneir_mcu_destroy(mcu);

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



int main(int argc, char *argv[])
{
    int exitcode = 0;
    int i;

    if (argc < 2)
    {
        printf("not enough arguments\n");
        exit(1);
    }

    if (setup(argv[1]) != 0) {
        perror("setup failed\n");
        exit(1);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL)
        exit(1);

    oneir_mcu_load_firmware(mcu, fp);

    fclose(fp);

    cleanup();

    return 0;
}
