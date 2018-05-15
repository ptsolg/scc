#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>

static void* xmalloc(size_t bytes, size_t alignment)
{
        void* block = malloc(bytes + alignment);
        return block 
                ? (void*)((char*)block + ((size_t)block % alignment))
                : 0;
}

#endif