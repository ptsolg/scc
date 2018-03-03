#ifndef SCC_CORE_HTAB_COMMON_H
#define SCC_CORE_HTAB_COMMON_H

#include "memory.h"

struct _htab
{
        uint8_t* _buckets;
        size_t _num_buckets;
        size_t _size;
        allocator* _alloc;
};

struct _hiter
{
        uint8_t* _bucket;
        const uint8_t* _end;
};

#endif