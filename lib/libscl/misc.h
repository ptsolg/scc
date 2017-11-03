#ifndef SMISC_H
#define SMISC_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <math.h>

#define SMIN(a, b) ((a) < (b) ? (a) : (b))
#define SMAX(a, b) ((a) > (b) ? (a) : (b))

#define MEM_FOREACH(PFIRST, PLAST, ITTYPE, ITNAME) \
        for (ITTYPE ITNAME = (ITTYPE)(PFIRST); \
                (char*)ITNAME < (char*)(PLAST); \
                ITNAME = (ITTYPE)((char*)ITNAME + sizeof(*ITNAME)))

// truncates v to next highest power of 2
static inline uint next_powerof2(uint v)
{
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        return v++;
}

// truncates v to previous highest power of 2
static inline uint prev_powerof2(uint v)
{
        return v ? next_powerof2(v) - 1 : 0;
}

static inline void* align_pointer(void* p, ssize alignment)
{
        return (void*)(((ssize)p + alignment - 1) & ~(ssize)(alignment - 1));
}

static inline ssize pointer_adjustment(void* p, ssize alignment)
{
        return (char*)align_pointer(p, alignment) - (char*)p;
}

// returns number of digits in n
static inline int ndigits(int n)
{
        return n ? (int)log10(n) + 1 : 1;
}

static inline suint64 mod2(suint64 x, uint pow)
{
        S_ASSERT(pow <= 64);
        if (pow == 64)
                return x;

        suint64 y = (((suint64)1) << pow);
        return (x & (y - 1));
}

#ifdef __cplusplus
}
#endif

#endif // !SMISC_H
