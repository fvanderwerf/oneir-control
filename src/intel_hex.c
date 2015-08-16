
#include "intel_hex.h" 

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ctype.h>
#include <errno.h>

#define CJMP(cond, label)       \
    do                          \
        if (cond)               \
            goto label;         \
    while(0)

#define CJMP_ERRNO(cond, label, newerrno)   \
    do {                                    \
        if (cond) {                         \
            goto label;                     \
            errno = newerrno;               \
        }                                   \
    } while(0)

#define NULL_ERROR(expr) CJMP((expr) == NULL, error)
#define NEG_ERROR(expr) CJMP((expr) < 0, error)


struct intel_hex
{
    int fd;
    char *data;
    size_t size;
    unsigned int readpos;
    void *buffer;
    int buffersize;
    uint8_t checksum;
};


intel_hex_t intel_hex_create(const char *filename)
{
    struct intel_hex *ihex = NULL;
    struct stat fileinfo;
    int fd = -1;
    void *map;

    NEG_ERROR(stat(filename, &fileinfo));

    NULL_ERROR(ihex = malloc(sizeof(struct intel_hex)));

    NEG_ERROR(ihex->fd = open(filename, O_RDONLY));

    CJMP(MAP_FAILED == (map = mmap(NULL, fileinfo.st_size, PROT_READ, MAP_SHARED, ihex->fd, 0)), error);

    ihex->fd = fd;
    ihex->data = map;
    ihex->size = fileinfo.st_size;
    ihex->readpos = 0;
    ihex->buffer = NULL;
    ihex->buffersize = 0;

    return ihex;

error:
    if (fd > 0)
        close(fd);

    if (ihex != NULL)
        free(ihex);

    return NULL;
}


static void intel_hex_skip_newline(intel_hex_t ihex)
{
    while(ihex->readpos < ihex->size && (
            ihex->data[ihex->readpos] == '\n' || ihex->data[ihex->readpos] == '\r'))
    {
        ihex->readpos++;
    }
}


static int intel_hex_read_start_code(intel_hex_t ihex)
{
    if (ihex->readpos < ihex->size && ihex->data[ihex->readpos] == ':') {
        ihex->readpos++;
        return 0;
    } else {
        errno = EINVAL;
        return -1;
    }
}


static int intel_hex_read_uint8(intel_hex_t ihex, uint8_t *value)
{
    int i;

    if ((ihex->size - ihex->readpos) < 2) {
        errno = EINVAL;
        goto error;
    }

    *value = 0;

    for (i = 0; i < 2; i++) {
        *value <<= 4;
        switch(ihex->data[ihex->readpos]) {
            case '0':
                break;
            case '1':
                *value += 1;
                break;
            case '2':
                *value += 2;
                break;
            case '3':
                *value += 3;
                break;
            case '4':
                *value += 4;
                break;
            case '5':
                *value += 5;
                break;
            case '6':
                *value += 6;
                break;
            case '7':
                *value += 7;
                break;
            case '8':
                *value += 8;
                break;
            case '9':
                *value += 9;
                break;
            case 'a':
            case 'A':
                *value += 10;
                break;
            case 'b':
            case 'B':
                *value += 11;
                break;
            case 'c':
            case 'C':
                *value += 12;
                break;
            case 'd':
            case 'D':
                *value += 13;
                break;
            case 'e':
            case 'E':
                *value += 14;
                break;
            case 'f':
            case 'F':
                *value += 15;
                break;
            default:
                errno = EINVAL;
                goto error;
                break;
        }
        ihex->readpos++;
    }

    ihex->checksum += *value;

    return 0;

error:
    return -1;
}


static int intel_hex_read_uint16(intel_hex_t ihex, uint16_t *value)
{
    uint8_t byte;

    NEG_ERROR(intel_hex_read_uint8(ihex, &byte));

    *value = byte << 8;

    NEG_ERROR(intel_hex_read_uint8(ihex, &byte));

    *value |= byte;

    return 0;
error:
    return -1;
}


int intel_hex_read_byte_string(intel_hex_t ihex, uint8_t *data, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        NEG_ERROR(intel_hex_read_uint8(ihex, data));
        data++;
    }

    return 0;

error:
    return -1;
}


int intel_hex_get_next(intel_hex_t ihex, struct intel_hex_record *record)
{
    uint8_t checksum, compchecksum;
    uint8_t type;
    int i;

    record->type = INTEL_HEX_EOF;
    record->address = 0;
    record->data = NULL;

    intel_hex_skip_newline(ihex);

    NEG_ERROR(intel_hex_read_start_code(ihex));

    /* reset checksum */
    ihex->checksum = 0;

    NEG_ERROR(intel_hex_read_uint8(ihex, &(record->size)));
    NEG_ERROR(intel_hex_read_uint16(ihex, &(record->address)));
    NEG_ERROR(intel_hex_read_uint8(ihex, &type));

    record->type = type;

    if (ihex->buffersize < record->size) {
        free(ihex->buffer);
        NULL_ERROR(ihex->buffer = malloc(record->size));
    }
    
    intel_hex_read_byte_string(ihex, ihex->buffer, record->size); 
    record->data = ihex->buffer;

    compchecksum = ihex->checksum;
    compchecksum = ~compchecksum;
    compchecksum++;

    NEG_ERROR(intel_hex_read_uint8(ihex, &checksum));

    CJMP_ERRNO(compchecksum != checksum, error, EINVAL);

    return 0;

error:
    return -1;
}


void intel_hex_destroy(intel_hex_t ihex)
{
    munmap(ihex->data, ihex->size);
    close(ihex->fd);
    free(ihex->buffer);
    free(ihex);
}

