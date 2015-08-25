
#ifndef ONEIR_MCU_H
#define ONEIR_MCU_H

#include "avr.h"

#include <stdio.h>

enum oneir_mode {
    ONEIR_NORMAL_MODE,
    ONEIR_PROG_MODE
};

typedef struct oneir_mcu *oneir_mcu_t;

oneir_mcu_t oneir_mcu_create(avr_t avr);

enum oneir_mode oneir_mcu_get_mode(oneir_mcu_t oneir);

int oneir_mcu_to_mode(oneir_mcu_t oneir, enum oneir_mode mode);

int oneir_mcu_load_firmware(oneir_mcu_t mcu, FILE *in);

int oneir_mcu_get_version(oneir_mcu_t oneir);

int oneir_mcu_send(oneir_mcu_t mcu, uint8_t address, uint8_t code);

void oneir_mcu_destroy(oneir_mcu_t oneir);


#endif /* ONEIR_MCU_H */
