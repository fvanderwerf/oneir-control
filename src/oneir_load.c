
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "gpio_smbus.h"
#include "sysfs_gpio.h"
#include "oneir_bus.h"
#include "override_gpio.h"
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
override_gpio_t override_sclk = NULL, override_miso = NULL, override_mosi = NULL;

sysfs_gpio_t smbclk = NULL, smbdat = NULL;
gpio_smbus_t smb = NULL;

gpio_spi_t spi = NULL;
oneir_bus_t bus = NULL;
avr_t avr = NULL;
oneir_mcu_t mcu = NULL;


void cleanup();


int setup(const char *filename)
{
    sclk = sysfs_gpio_create(11);
    miso = sysfs_gpio_create(9);
    mosi = sysfs_gpio_create(10);
    reset = sysfs_gpio_create(25);

    CGE(sclk == NULL || miso == NULL || mosi == NULL || reset == NULL);

    override_sclk = override_gpio_create(sysfs_gpio_to_gpio(sclk));
    override_miso = override_gpio_create(sysfs_gpio_to_gpio(miso));
    override_mosi = override_gpio_create(sysfs_gpio_to_gpio(mosi));

    spi = gpio_spi_create(
            override_gpio_to_gpio(override_sclk),
            override_gpio_to_gpio(override_mosi),
            override_gpio_to_gpio(override_miso));

    if (spi == NULL)
        goto error;

    CGE_NULL(bus = oneir_bus_create(override_sclk, override_miso, override_mosi));

    CGE_NULL(smbdat = sysfs_gpio_create(2));
    CGE_NULL(smbclk = sysfs_gpio_create(3));

    CGE_NULL(smb = gpio_smbus_create(sysfs_gpio_to_gpio(smbclk), sysfs_gpio_to_gpio(smbdat)));

    avr = avr_create(spi, sysfs_gpio_to_gpio(reset));
    if (avr == NULL)
        goto error;

    CGE_NULL(mcu = oneir_mcu_create(avr, bus, smb));

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

    if (smb != NULL)
        gpio_smbus_destroy(smb);

    if (smbclk != NULL)
        sysfs_gpio_destroy(smbclk);

    if (smbdat != NULL)
        sysfs_gpio_destroy(smbdat);

    if (bus != NULL)
        oneir_bus_destroy(bus);

    if (override_sclk != NULL)
        override_gpio_destroy(override_sclk);

    if (override_miso != NULL)
        override_gpio_destroy(override_miso);

    if (override_mosi != NULL)
        override_gpio_destroy(override_mosi);

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
    FILE *fp;
    int exitcode = 0;
    int i;


    if (setup(argv[1]) != 0) {
        perror("setup failed\n");
        exit(1);
    }

    if (argc == 2)
    {

        CGE_NULL(fp = fopen(argv[1], "r"));

        CGE_NEG(oneir_mcu_load_firmware(mcu, fp));

        fclose(fp);
        printf("programming completed\n");
    }

    if (argc == 3)
    {
        uint8_t address, code;
        sleep(1);

        address = atoi(argv[1]);
        code = atoi(argv[2]);

        oneir_mcu_send(mcu, address, code);
    }

    cleanup();

    return 0;

error:
    return -1;
}
