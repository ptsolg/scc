#include "scc/core/malloc.h"
#include <stdlib.h>

malloc_fn smalloc = &malloc;
free_fn sfree = &free;

extern void scl_override_malloc(malloc_fn malloc, free_fn free)
{
        smalloc = malloc;
        sfree = free;
}