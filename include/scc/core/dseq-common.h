#ifndef SCC_CORE_DSEQ_COMMON_H
#define SCC_CORE_DSEQ_COMMON_H

#include "memory.h"

struct _dseq
{
        suint8* _elems;
        allocator* _alloc;
        ssize _size;
        ssize _capacity;
};

#endif