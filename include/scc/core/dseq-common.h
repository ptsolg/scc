#ifndef SCC_CORE_DSEQ_COMMON_H
#define SCC_CORE_DSEQ_COMMON_H

#include "memory.h"

struct _dseq
{
        uint8_t* _elems;
        allocator* _alloc;
        size_t _size;
        size_t _capacity;
};

#endif