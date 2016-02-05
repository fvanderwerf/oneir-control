
#ifndef HEX_H
#define HEX_H

#include <stdint.h>
#include <stddef.h>

int hex_decode(uint8_t *dst, const char *src, size_t srclen);

#endif /* HEX_H */
