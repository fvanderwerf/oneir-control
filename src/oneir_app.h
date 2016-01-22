
#ifndef ONEIR_APP
#define ONEIR_APP

#include "oneir_mcu.h"
#include "avr.h"
#include "oneir_bus.h"
#include "gpio_smbus.h"
#include "gpio_spi.h"
#include "override_gpio.h"
#include "sysfs_gpio.h"

struct oneir_app_config {
    int gpio_sclk;
    int gpio_miso;
    int gpio_mosi;
    int gpio_reset;
    int gpio_smbdat;
    int gpio_smbclk;
};

struct oneir_app {
    sysfs_gpio_t sclk;
    sysfs_gpio_t miso;
    sysfs_gpio_t mosi;
    sysfs_gpio_t reset;

    override_gpio_t override_sclk;
    override_gpio_t override_miso;
    override_gpio_t override_mosi;

    sysfs_gpio_t smbclk;
    sysfs_gpio_t smbdat;

    gpio_smbus_t smb;

    gpio_spi_t spi;
    oneir_bus_t bus;
    avr_t avr;
    oneir_mcu_t mcu;
};


struct oneir_app *construct_app(const struct oneir_app_config *config);

void destroy_app(struct oneir_app *app);

#endif /* ONEIR_APP */
