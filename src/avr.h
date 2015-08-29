
#ifndef AVR_H
#define AVR_H

#include "gpio.h"
#include "gpio_spi.h"

#include <stdint.h>

enum attiny {
    ATTINY_UNKNOWN,
    ATTINY25,
    ATTINY45,
    ATTINY85
};

typedef struct avr *avr_t;

avr_t avr_create(gpio_spi_t spi, gpio_t reset);

int avr_reset(avr_t avr);

int avr_unreset(avr_t avr);

int avr_program_enable(avr_t avr);

enum attiny avr_get_type(avr_t avr);

int avr_poll_ready(avr_t avr);

int avr_chip_erase(avr_t avr);

int avr_write_flash_load(avr_t avr, uint16_t offset, uint16_t value);

int avr_write_flash_page(avr_t avr, uint16_t address);

int avr_read_flash(avr_t avr, uint16_t address, uint16_t *value);

void avr_destroy(avr_t avr);

#endif /* AVR_H */
