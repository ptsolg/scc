#ifndef SCC_CORE_COMMON_H
#define SCC_CORE_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if defined(_MSC_VER)
#define S_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
#define S_GCC 1
#elif defined(__clang__)
#define S_CLANG 1
#else
#error Unknown compiller.
#endif

#if defined(_WIN32) || defined(_WIN64)
#define S_WIN 1
#define S_MAX_PATH_LEN 256
#define S_PATH_DELIMETER '\\'
#elif defined(__APPLE__)
#define S_OSX 1
#define S_MAX_PATH_LEN 256
#define S_PATH_DELIMETER '/'
#else
#error Unknown OS.
#endif

#if INTPTR_MAX == INT32_MAX
#define S_X32 1
#else
#define S_X64 1
#endif

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

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef long double ldouble;

#endif
