#ifndef SCC_CORE_HTAB_COMMON_H
#define SCC_CORE_HTAB_COMMON_H

#include "memory.h"

struct _htab
{
        uint8_t* _buckets;
        ssize _num_buckets;
        ssize _size;
        allocator* _alloc;
};

struct _hiter
{
        suint8* _bucket;
        const suint8* _end;
};

#endif