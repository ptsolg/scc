#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

void* alloc(size_t size);
void dealloc(void* block);

#endif
