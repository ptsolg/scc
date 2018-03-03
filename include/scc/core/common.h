#ifndef SCC_CORE_COMMON_H
#define SCC_CORE_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#if defined(_MSC_VER)
#define COMPILER_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
#define COMPILER_GCC 1
#elif defined(__clang__)
#define COMPILER_CLANG 1
#else
#error Unknown compiller.
#endif

#if defined(_WIN32) || defined(_WIN64)
#define OS_WIN 1
#define MAX_PATH_LEN 256
#define PATH_DELIMETER '\\'
#elif defined(__APPLE__)
#define OS_OSX 1
#define MAX_PATH_LEN 256
#define PATH_DELIMETER '/'
#else
#error Unknown OS.
#endif

#define MAX_CMD_SIZE 4096

#if INTPTR_MAX == INT32_MAX
#define TARGET_X32 1
#else
#define TARGET_X64 1
#endif

#if _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#define UNREACHABLE() \
        do { \
                volatile int unreachable = 0; \
                assert(unreachable); \
        } while (0)

#if COMPILER_MSVC || COMPILER_CLANG || COMPILER_GCC
#else
#error // todo static_assert
#endif

#define ARRAY_SIZE(A) (sizeof(A) / sizeof(*(A)))

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef long double ldouble;

#endif
