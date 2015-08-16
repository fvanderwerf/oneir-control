
#ifndef INTEL_HEX_H
#define INTEL_HEX_H

#include <stdint.h>

enum intel_hex_record_type {
    INTEL_HEX_DATA,
    INTEL_HEX_EOF
};

typedef struct intel_hex *intel_hex_t;

struct intel_hex_record
{
    enum intel_hex_record_type type;
    uint16_t address;
    uint8_t size;
    uint8_t padding;
    void *data;
};

intel_hex_t intel_hex_create(const char *filename);

int intel_hex_get_next(intel_hex_t ihex, struct intel_hex_record *record);

void intel_hex_destroy(intel_hex_t ihex);

#endif /* INTEL_HEX_H */

