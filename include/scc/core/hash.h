#ifndef SHASH_H
#define SHASH_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "bit-utils.h"

static inline suint32 murmurhash3_86_32(const void* data, suint32 seed, int len)
{
        const suint8* bytes = (const suint8*)data;
        const int nblocks = len / 4;
        int i;

        suint32 h1 = seed;

        suint32 c1 = 0xcc9e2d51;
        suint32 c2 = 0x1b873593;

        const suint32* blocks = (const suint32*)(bytes + nblocks * 4);
        for (i = -nblocks; i; i++)
        {
                suint32 k1 = blocks[i];

                k1 *= c1;
                k1 = s_rotl32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = s_rotl32(h1, 13);
                h1 = h1 * 5 + 0xe6546b64;
        }

        const suint8* tail = (const suint8*)(bytes + nblocks * 4);
        suint32 k1 = 0;

        switch (len & 3)
        {
                case 3: k1 ^= tail[2] << 16;
                case 2: k1 ^= tail[1] << 8;
                case 1: k1 ^= tail[0];

                        k1 *= c1;
                        k1 = s_rotl32(k1, 15);
                        k1 *= c2;
                        h1 ^= k1;
        }

        h1 ^= len;
        h1 = s_mix32(h1);
        return h1;
}

#ifdef __cplusplus
}
#endif

#endif // !SHASH_H
