
bin_PROGRAMS = oneir_load oneird

oneir_load_SOURCES = oneir_load.c sysfs_gpio.c gpio_spi.c avr.c intel_hex.c override_gpio.c oneir_bus.c oneir_mcu.c gpio_smbus.c

oneird_SOURCES = oneird.c oneir_app.c sysfs_gpio.c gpio_spi.c avr.c override_gpio.c oneir_bus.c oneir_mcu.c gpio_smbus.c
oneird_CFLAGS = @JSONC_CFLAGS@
oneird_LDADD = @JSONC_LIBS@
