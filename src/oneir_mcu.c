
#include "oneir_mcu.h"

#include "cge.h"
#include "gpio_smbus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static const int i2c_slave_addr = 0x10;

struct oneir_mcu
{
    avr_t avr;
    oneir_bus_t bus;
    gpio_smbus_t smbus;
};

oneir_mcu_t oneir_mcu_create(avr_t avr, oneir_bus_t bus, gpio_smbus_t smbus)
{
    struct oneir_mcu *mcu = NULL;

    CGE_NULL(mcu = malloc(sizeof(struct oneir_mcu)));

    mcu->avr = avr;
    mcu->bus = bus;
    mcu->smbus = smbus; 

    CGE_NEG(avr_reset(avr));
    CGE_NEG(avr_unreset(avr));
    
    return mcu;

error:
    if (mcu)
        free(mcu);

    return NULL;
}


int oneir_mcu_load_firmware(oneir_mcu_t oneir, FILE *in)
{
    uint16_t value;
    uint16_t address = 0;
    uint16_t checksum1 = 0;
    uint16_t checksum2 = 0;
    uint16_t actualchecksum1 = 0;
    uint16_t actualchecksum2 = 0;
    int firmwaresize = 0;
    int i;

    oneir_bus_select(oneir->bus, ONEIR_SPI);

    CGE_NEG(avr_reset(oneir->avr));

    CGE_NEG(avr_program_enable(oneir->avr));

    CGE_NEG(avr_chip_erase(oneir->avr));

    do {
        if (fread(&value, sizeof(value), 1, in) == 1) {
            value = le16toh(value);
            checksum1 += value;
            checksum2 += checksum2 + checksum1;
            firmwaresize++;

            CGE_NEG(avr_write_flash_load(oneir->avr, address, value));
        } else {
            CGE(ferror(in));
            break;
        }

        address++;

        if (!(address % 32))
            CGE_NEG(avr_write_flash_page(oneir->avr, address - 32));
    } while(1);

    if (address % 32)
        CGE_NEG(avr_write_flash_page(oneir->avr, address & 0xFFE0));

    for (i = 0; i < firmwaresize; i++) {
        CGE_NEG(avr_read_flash(oneir->avr, i, &value));
        actualchecksum1 += value;
        actualchecksum2 += actualchecksum2 + actualchecksum1;
    }

    if (actualchecksum1 != checksum1 || actualchecksum2 != checksum2) {
        GE();
    }

    CGE_NEG(avr_unreset(oneir->avr));

    return 0;

error:
    return -1;
}

int oneir_mcu_get_version(oneir_mcu_t oneir)
{
error:
    return -1;
}

int oneir_mcu_send_rc5(oneir_mcu_t oneir, uint8_t address, uint8_t code)
{
    oneir_bus_select(oneir->bus, ONEIR_I2C);

    uint16_t value = address << 8 | code;
    uint8_t buf[] = { 0x03, 0x01, address, code };

    CGE_NEG(gpio_smbus_write_buffer(oneir->smbus, i2c_slave_addr, buf, sizeof(buf)));

    return 0;

error:
    perror("oneir_mcu_send:");
    return -1;
}

int oneir_mcu_send_raw(oneir_mcu_t oneir, uint8_t *command, size_t len)
{
    struct iovec vector[2];
    uint8_t msglen = len;

    oneir_bus_select(oneir->bus, ONEIR_I2C);

    vector[0].iov_base = &msglen;
    vector[0].iov_len = 1;
    vector[1].iov_base = command;
    vector[1].iov_len = len;
    CGE_NEG(gpio_smbus_write_vector(oneir->smbus, i2c_slave_addr, vector, 2));

    return 0;
error:
    perror("oneir_mcu_send:");
    return -1;
}

void oneir_mcu_destroy(oneir_mcu_t oneir)
{
    free(oneir);
}


