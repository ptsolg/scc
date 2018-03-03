#ifndef SCC_CORE_HASH_H
#define SCC_CORE_HASH_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

static inline uint32_t rotl32(uint32_t v, uint32_t n)
{
        return (v << n) | (v >> (32 - n));
}

static inline uint32_t mix32(uint32_t v)
{
        v ^= v >> 16;
        v *= 0x85ebca6b;
        v ^= v >> 13;
        v *= 0xc2b2ae35;
        v ^= v >> 16;

        return v;
}

static inline uint32_t murmurhash3_86_32(const void* data, uint32_t seed, int len)
{
        const uint8_t* bytes = (const uint8_t*)data;
        const int nblocks = len / 4;
        int i;

        uint32_t h1 = seed;

        uint32_t c1 = 0xcc9e2d51;
        uint32_t c2 = 0x1b873593;

        const uint32_t* blocks = (const uint32_t*)(bytes + nblocks * 4);
        for (i = -nblocks; i; i++)
        {
                uint32_t k1 = blocks[i];

                k1 *= c1;
                k1 = rotl32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = rotl32(h1, 13);
                h1 = h1 * 5 + 0xe6546b64;
        }

        const uint8_t* tail = (const uint8_t*)(bytes + nblocks * 4);
        uint32_t k1 = 0;

        switch (len & 3)
        {
                case 3: k1 ^= tail[2] << 16;
                case 2: k1 ^= tail[1] << 8;
                case 1: k1 ^= tail[0];

                        k1 *= c1;
                        k1 = rotl32(k1, 15);
                        k1 *= c2;
                        h1 ^= k1;
        }

        h1 ^= len;
        h1 = mix32(h1);
        return h1;
}

#ifdef __cplusplus
}
#endif

#endif
