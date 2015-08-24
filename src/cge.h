
#ifndef CGE_H
#define CGE_H

#include <errno.h>

#define GE()                    \
    do                          \
        goto error;             \
    while(0)

#define CGE(cond)               \
    do                          \
        if (cond)               \
            goto error;         \
    while(0)

#define GE_ERRNO(newerrno)                  \
    do {                                    \
        errno = newerrno;                   \
        goto error;                         \
    } while(0)

#define CGE_ERRNO(cond, newerrno)           \
    do {                                    \
        if (cond) {                         \
            errno = newerrno;               \
            goto error;                     \
        }                                   \
    } while(0)

#define CGE_NULL(expr) CGE((expr) == NULL)
#define CGE_NEG(expr) CGE((expr) < 0)

#endif /* CGE_H */
