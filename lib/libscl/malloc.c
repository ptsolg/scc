#include "malloc.h"
#include <stdlib.h>

extern malloc_fn smalloc = &malloc;
extern free_fn   sfree   = &free;

extern void scl_override_malloc(malloc_fn malloc, free_fn free)
{
        smalloc = malloc;
        sfree   = free;
}