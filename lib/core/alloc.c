#include "scc/core/alloc.h"

#include <stdlib.h>
#include <stdio.h>

void* alloc(size_t size)
{
        void* p = malloc(size);
        if (!p)         {
                fprintf(stderr, "Out of memory.");
                exit(-1);
        }
        return p;
}

void dealloc(void* block)
{
        free(block);
}
