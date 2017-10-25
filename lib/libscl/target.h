#ifndef STARGET_H
#define STARGET_H

#ifdef __cplusplus
#define S_HAS_PRAGMA
#pragma once
#endif

#include <stdint.h>

#if defined(_MSC_VER)
        #define S_MSVC 1
#elif defined(__GNUC__) || defined(__GNUG__)
        #define S_GCC  1
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

#endif // !STARGET_H