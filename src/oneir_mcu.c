
#include "oneir_mcu.h"

#include "cge.h"

#include <linux/i2c-dev-user.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct oneir_mcu
{
    avr_t avr;
    oneir_bus_t bus;
    int i2cfd;
};

oneir_mcu_t oneir_mcu_create(avr_t avr, oneir_bus_t bus)
{
    struct oneir_mcu *mcu;

    CGE_NULL(mcu = malloc(sizeof(struct oneir_mcu)));

    mcu->avr = avr;
    mcu->bus = bus;
    mcu->i2cfd = -1;
    
    CGE_NEG(mcu->i2cfd = open("/dev/i2c-1", O_RDWR));

    int addr = 0x40; /* The I2C address */

    CGE_NEG(ioctl(mcu->i2cfd, I2C_SLAVE, addr) < 0);

    return mcu;

error:

    if (mcu->i2cfd > 0)
        close(mcu->i2cfd);
    
    return NULL;
}


int oneir_mcu_load_firmware(oneir_mcu_t oneir, FILE *in)
{
    uint16_t value;
    uint16_t address = 0;

    oneir_bus_select(oneir->bus, ONEIR_SPI);

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
error:
    return -1;
}

int oneir_mcu_send(oneir_mcu_t oneir, uint8_t address, uint8_t code)
{
    oneir_bus_select(oneir->bus, ONEIR_I2C);

    __u8 reg = 0x10;
    __u16 value = address << 8 | code;
    __s32 result;

    CGE_NEG(result = i2c_smbus_write_word_data(oneir->i2cfd, reg, value));

error:
    perror("oneir_mcu_send:");
    return -1;
}

void oneir_mcu_destroy(oneir_mcu_t oneir)
{
    close(oneir->i2cfd);
    free(oneir);
}


