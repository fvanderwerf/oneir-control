
#include "oneir_app.h"


#include "cge.h"
#include <stdlib.h>


struct oneir_app *construct_app(const struct oneir_app_config *config)
{
    struct oneir_app *app = NULL;

    CGE_NULL(app = malloc(sizeof(struct oneir_app)));

    app->sclk = app->miso = app->mosi = app->reset = NULL;
    app->override_sclk = app->override_miso = app->override_mosi = NULL;
    app->smbclk = app->smbdat = NULL;
    app->smb = NULL;
    app->spi = NULL;
    app->avr = NULL;
    app->mcu = NULL;

    app->sclk = sysfs_gpio_create(config->gpio_sclk);
    app->miso = sysfs_gpio_create(config->gpio_miso);
    app->mosi = sysfs_gpio_create(config->gpio_mosi);
    app->reset = sysfs_gpio_create(config->gpio_reset);

    CGE(app->sclk == NULL || app->miso == NULL || app->mosi == NULL || app->reset == NULL);

    app->override_sclk = override_gpio_create(sysfs_gpio_to_gpio(app->sclk));
    app->override_miso = override_gpio_create(sysfs_gpio_to_gpio(app->miso));
    app->override_mosi = override_gpio_create(sysfs_gpio_to_gpio(app->mosi));

    CGE(app->override_sclk == NULL || app->override_miso == NULL || app->override_mosi == NULL);

    CGE_NULL(app->spi = gpio_spi_create(
            override_gpio_to_gpio(app->override_sclk),
            override_gpio_to_gpio(app->override_mosi),
            override_gpio_to_gpio(app->override_miso)));

    CGE_NULL(app->bus = oneir_bus_create(
                app->override_sclk,
                app->override_miso,
                app->override_mosi));

    CGE_NULL(app->smbdat = sysfs_gpio_create(config->gpio_smbdat));
    CGE_NULL(app->smbclk = sysfs_gpio_create(config->gpio_smbclk));

    CGE_NULL(app->smb = gpio_smbus_create(
                sysfs_gpio_to_gpio(app->smbclk),
                sysfs_gpio_to_gpio(app->smbdat)));

    CGE_NULL(app->avr = avr_create(app->spi, sysfs_gpio_to_gpio(app->reset)));

    CGE_NULL(app->mcu = oneir_mcu_create(app->avr, app->bus, app->smb));

    return app;

error:
    destroy_app(app);
    return NULL;
}


void destroy_app(struct oneir_app *app)
{
    if (app != NULL) {
        free(app);
    }
}
