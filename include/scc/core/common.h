#ifndef SCC_CORE_COMMON_H
#define SCC_CORE_COMMON_H

#include "target.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if _DEBUG
#define S_DEBUG 1
#define S_ASSERT(e) assert(e)
#else
#define S_DEBUG 0
#define S_ASSERT(e) 1
#endif

#define S_ASSERT_PTR(ptr) S_ASSERT(ptr)

#define S_UNREACHABLE() \
        do { \
                volatile int unreachable = 0; \
                S_ASSERT(unreachable); \
        } while (0)

#if S_MSVC || S_CLANG || S_GCC

#define S_STATIC_ASSERT(e, msg) static_assert(e, msg)

#else
#error // todo
#endif

#define S_ARRAY_SIZE(A) (sizeof(A) / sizeof(*(A)))

typedef signed char sint8;
typedef unsigned char suint8;
typedef signed short sint16;
typedef unsigned short suint16;
typedef int sint32;
typedef unsigned int suint32;
typedef long long sint64;
typedef unsigned long long suint64;

#if S_X32
typedef suint32 ssize;
typedef sint32 sptrdiff;
#else
typedef suint64 ssize;
typedef sint64 sptrdiff;
#endif

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef long double ldouble;

#endif
