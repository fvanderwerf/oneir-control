
#include "oneir_mcu.h"

#include "cge.h"

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev-user.h>

struct oneir_mcu
{
    avr_t avr;
    enum oneir_mode mode;
    int i2cfd;
};

oneir_mcu_t oneir_mcu_create(avr_t avr)
{
    struct oneir_mcu *mcu;

    CGE_NULL(mcu = malloc(sizeof(struct oneir_mcu)));

    mcu->avr = avr;
    mcu->mode = ONEIR_NORMAL_MODE;

    return mcu;

error:
    return NULL;
}


enum oneir_mode oneir_mcu_get_mode(oneir_mcu_t oneir)
{
    return oneir->mode;
}


int oneir_mcu_to_mode(oneir_mcu_t oneir, enum oneir_mode mode)
{
    switch(mode) {
        case ONEIR_NORMAL_MODE:
            if (oneir->mode == ONEIR_NORMAL_MODE)
                break;

            if (oneir->mode == ONEIR_PROG_MODE) {

            }

            break;
        case ONEIR_PROG_MODE:
            break;
        default:
            GE_ERRNO(EINVAL);
            break;
    }

    oneir->mode = mode;
    return 0;
error:
    return -1;
}

int oneir_mcu_load_firmware(oneir_mcu_t oneir, FILE *in)
{
    uint16_t value;
    uint16_t address = 0;

    //CGE_ERRNO(oneir->mode != ONEIR_PROG_MODE, EPERM);

    CGE_NEG(avr_reset(oneir->avr));

    CGE_NEG(avr_program_enable(oneir->avr));

    do {
        if (fread(&value, sizeof(value), 1, in) == 1) {
            value = le16toh(value);
            CGE_NEG(avr_write_flash_load(oneir->avr, address, value) < 0);
        } else {
            CGE(ferror(in));
            break;
        }

        address++;

        if (!(address % 32))
            CGE_NEG(avr_write_flash_page(oneir->avr, address) < 0);
    } while(1);

    if (address % 32)
        CGE_NEG(avr_write_flash_page(oneir->avr, address) < 0);

    avr_unreset(oneir->avr);

    return 0;

error:
    return -1;
}

int oneir_mcu_get_version(oneir_mcu_t oneir)
{
    CGE_ERRNO(oneir->mode != ONEIR_NORMAL_MODE, EPERM);

    return 0;

error:
    return -1;
}

int oneir_mcu_send(oneir_mcu_t oneir, uint8_t address, uint8_t code)
{
    CGE_ERRNO(oneir->mode != ONEIR_NORMAL_MODE, EPERM);

    return 0;

error:
    return -1;
}

void oneir_mcu_destroy(oneir_mcu_t oneir)
{
    free(oneir);
}


