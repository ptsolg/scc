#ifndef SBIT_UTILS_H
#define SBIT_UTILS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

static inline suint32 s_rotl32(suint32 v, suint32 n)
{
        return (v << n) | (v >> (32 - n));
}

static inline suint32 s_mix32(suint32 v)
{
        v ^= v >> 16;
        v *= 0x85ebca6b;
        v ^= v >> 13;
        v *= 0xc2b2ae35;
        v ^= v >> 16;

        return v;
}

#ifdef __cplusplus
}
#endif

#endif // !SBIT_UTILS_H
