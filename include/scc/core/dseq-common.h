#ifndef SDSEQ_COMMON_H
#define SDSEQ_COMMON_H

#include "memory.h"

struct _dseq
{
        suint8* _elems;
        allocator* _alloc;
        ssize _size;
        ssize _capacity;
};

#endif