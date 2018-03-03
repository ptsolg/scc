#ifndef SCC_CORE_MISC_H
#define SCC_CORE_MISC_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <math.h>

#undef MIN
#undef MAX

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

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

#define IS_POWEROF2(X) ((X) && !((X) & ((X) - 1)))

static inline void* align_pointer(void* p, size_t alignment)
{
        return (void*)(((size_t)p + alignment - 1) & ~(size_t)(alignment - 1));
}

static inline size_t pointer_adjustment(void* p, size_t alignment)
{
        return (char*)align_pointer(p, alignment) - (char*)p;
}

// returns number of digits in n
static inline int ndigits(int n)
{
        return n ? (int)log10(n) + 1 : 1;
}

static inline uint64_t mod2(uint64_t x, uint pow)
{
        assert(pow <= 64);
        if (pow == 64)
                return x;

        uint64_t y = (((uint64_t)1) << pow);
        return (x & (y - 1));
}

typedef enum
{
        CR_EQ,
        CR_LE,
        CR_GR,
} cmp_result;

#define SORT_MAX_OBJECT_SIZE 1024

extern void sort(void* data, size_t n, size_t obsize,
        cmp_result(*const cmp_fn)(void*, const void*, const void*), void* ex_data);

#ifdef __cplusplus
}
#endif

#endif
