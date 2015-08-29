
#include "avr.h"

#include "cge.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ATTINY_FLASH_WORDSIZE 2
#define ATTINY_EEPROM_WORDSIZE 1

struct attiny_config {
    enum attiny type;
    size_t flash_size;      //in number of words
    size_t flash_pagesize;  //in number of words
    size_t flash_nopages;   //number of pages

    size_t eeprom_size;     //in number of words
    size_t eeprom_pagesize; //in number of words
    size_t eeprom_nopages;  //number of pages
};

struct avr {
    gpio_spi_t spi;
    gpio_t reset;

    const struct attiny_config *config;
};

const struct attiny_config attiny25_config = {
    .type = ATTINY25,

    .flash_size = 1024,
    .flash_pagesize = 16,
    .flash_nopages = 64,

    .eeprom_size = 128,
    .eeprom_pagesize = 4,
    .eeprom_nopages = 32
};

const struct attiny_config attiny45_config = {
    .type = ATTINY45,

    .flash_size = 2048,
    .flash_pagesize = 32,
    .flash_nopages = 64,

    .eeprom_size = 256,
    .eeprom_pagesize = 4,
    .eeprom_nopages = 64
};

const struct attiny_config attiny85_config = {
    .type = ATTINY85,

    .flash_size = 4096,
    .flash_pagesize = 32,
    .flash_nopages = 128,

    .eeprom_size = 512,
    .eeprom_pagesize = 4,
    .eeprom_nopages = 128
};

static enum attiny avr_read_signature(avr_t avr);

avr_t avr_create(gpio_spi_t spi, gpio_t reset)
{
    struct avr *avr;
   
    CGE_NULL(avr = malloc(sizeof(struct avr)));

    CGE_NEG(gpio_set_direction(reset, GPIO_OUT) != 0);

    avr->spi = spi;
    avr->reset = reset;
    avr->config = NULL;

    return avr;

error:
    if (avr != NULL)
        free(avr);

    return NULL;
}

int avr_reset(avr_t avr)
{
    CGE_NEG(sysfs_gpio_write(avr->reset, 0) != 0);

    usleep(1000);

    CGE_NEG(sysfs_gpio_write(avr->reset, 1) != 0);
        
    usleep(1000);

    CGE_NEG(sysfs_gpio_write(avr->reset, 0) != 0);

    usleep(20000);

    return 0;

error:
    return -1;
}

int avr_unreset(avr_t avr)
{
    return sysfs_gpio_write(avr->reset, 1) != 0;
}


int avr_program_enable(avr_t avr)
{
    char *tx = "\xac\x53\x00\x00";
    char rx[4];
    enum attiny type;

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4) != 0);
    CGE_ERRNO(rx[2] != '\x53', EPROTO);

    CGE_NEG(type = avr_read_signature(avr));
    switch (type){
        case ATTINY25:
            avr->config = &attiny25_config;
            break;
        case ATTINY45:
            avr->config = &attiny45_config;
            break;
        case ATTINY85:
            avr->config = &attiny85_config;
            break;
        default:
            avr->config = NULL;
            GE_ERRNO(EPROTO);
            break;
    }

    return 0;
error:
    return -1;
}


static enum attiny avr_parse_signature(const char *signature)
{
    if (memcmp(signature, "\x1e\x91\x08", 3) == 0)
        return ATTINY25;
    else if (memcmp(signature, "\x1e\x92\x06", 3) == 0)
        return ATTINY45;
    else if (memcmp(signature, "\x1e\x93\x0b", 3) == 0)
        return ATTINY85;
    else
        return ATTINY_UNKNOWN;
}


static enum attiny avr_read_signature(avr_t avr)
{
    const char *tx;
    char rx[4];
    char signature[3];

    tx = "\x30\x00\x00\x00";
    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4) != 0);

    signature[0] = rx[3];

    tx = "\x30\x00\x01\x00";
    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4) != 0);

    signature[1] = rx[3];

    tx = "\x30\x00\x02\x00";
    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4) != 0);

    signature[2] = rx[3];

    return avr_parse_signature(signature);

error:
    return -1;
}

enum attiny avr_get_type(avr_t avr)
{
    CGE_ERRNO(avr->config == NULL, EINVAL);
    return avr->config->type;

error:
    return -1;
}


int avr_poll_ready(avr_t avr)
{
    const char *tx = "\xf0\x00\x00\x00";
    char rx[4];

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4) == -1);

    return rx[3] & 0x01;

error:
    return -1;
}


int avr_chip_erase(avr_t avr)
{
    const char *tx = "\xac\x80\x00\x00";
    char rx[4];

    return gpio_spi_transfer(avr->spi, tx, rx, 4);
}

int avr_write_flash_load(avr_t avr, uint16_t offset, uint16_t value)
{
    uint8_t tx[4];
    uint8_t rx[4];

    offset = offset & (avr->config->flash_pagesize - 1);

    tx[0] = '\x40';
    tx[1] = (offset >> 8) & 0xff;
    tx[2] = offset & 0xff;
    tx[3] = value & 0xff;

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4));

    tx[0] = 0x48;
    tx[3] = (value >> 8) & 0xff;

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4));

    return 0;
error:

    return -1;
}


int avr_write_flash_page(avr_t avr, uint16_t address)
{
    uint8_t tx[4];
    uint8_t rx[4];

    address &= ~(avr->config->flash_pagesize - 1);

    tx[0] = 0x4c;
    tx[1] = (address >> 8) & 0xFF;
    tx[2] = address & 0xFF;
    tx[3] = 0x00;

    return gpio_spi_transfer(avr->spi, tx, rx, 4);
}

int avr_read_flash(avr_t avr, uint16_t address, uint16_t *value)
{
    uint8_t tx[] = { 0x28, (address >> 8) & 0xFF, address & 0xFF, 0x0 };
    uint8_t rx[4];

    *value = 0;

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4));

    *value = (rx[3] << 8) & 0xFF00;

    tx[0] = 0x20;

    CGE_NEG(gpio_spi_transfer(avr->spi, tx, rx, 4));

    *value |= rx[3] & 0xFF;

    return 0;
error:
    return -1;
}

void avr_destroy(avr_t avr)
{
    free(avr);
}
