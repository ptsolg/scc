#include "scc/core/malloc.h"
#include <stdlib.h>

malloc_fn core_malloc = &malloc;
free_fn core_free = &free;

extern void override_malloc(malloc_fn malloc, free_fn free)
{
        core_malloc = malloc;
        core_free = free;
}