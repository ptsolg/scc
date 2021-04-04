#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define UNREACHABLE() \
        do { \
                volatile int unreachable = 0; \
                assert(unreachable); \
        } while (0)

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*(A)))

typedef unsigned int uint;
typedef long double ldouble;

typedef int errcode;

#define EC_NO_ERROR ((errcode)0)
#define EC_ERROR ((errcode)1)

#define EC_SUCCEEDED(E) (((errcode)(E)) == EC_NO_ERROR)
#define EC_FAILED(E) (!EC_SUCCEEDED(E))

#endif
