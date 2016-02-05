
#include "hex.h"

#include "cge.h"

static int hex_decode_char(char c)
{
    switch (c) {
        case '0':
            return 0;
            break;
        case '1':
            return 1;
            break;
        case '2':
            return 2;
            break;
        case '3':
            return 3;
            break;
        case '4':
            return 4;
            break;
        case '5':
            return 5;
            break;
        case '6':
            return 6;
            break;
        case '7':
            return 7;
            break;
        case '8':
            return 8;
            break;
        case '9':
            return 9;
            break;
        case 'a':
        case 'A':
            return 10;
            break;
        case 'b':
        case 'B':
            return 11;
            break;
        case 'c':
        case 'C':
            return 12;
            break;
        case 'd':
        case 'D':
            return 13;
            break;
        case 'e':
        case 'E':
            return 14;
            break;
        case 'f':
        case 'F':
            return 15;
            break;
        default:
            GE_ERRNO(EINVAL);
    }
error:
    return -1;
}

int hex_decode(uint8_t *dst, const char *src, size_t srclen)
{
    int i;

    CGE_ERRNO(srclen % 2, EINVAL);

    for (i = 0; i < srclen/2; i++) {
        int msc, lsc;
        CGE_NEG(msc = hex_decode_char(src[0]));
        CGE_NEG(lsc = hex_decode_char(src[1]));

        *dst = msc << 4 | lsc;

        src += 2;
        dst++;
    }

    return 0;
error:
    return -1;
}

