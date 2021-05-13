#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifdef __SCC__
#define ___scc_concat(x, y) x ## y
#define __scc_concat(x, y) ___scc_concat(x, y)
#define static_assert(x, y) static char __scc_concat(static_assert_, __LINE__)[(x) && y]
#else
#define static_assert(x, y) _Static_assert(x, y)
#endif

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
