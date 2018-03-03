#ifndef SCC_CORE_ALLOCATOR_H
#define SCC_CORE_ALLOCATOR_H

#include "common.h"
#include "misc.h"

typedef void*(*allocate_fn)(void*, ssize);
typedef void*(*allocate_aligned_fn)(void*, ssize, ssize);
typedef void(*deallocate_fn)(void*, void*);

// an allocator for 'heavy' allocations that is used by various data structures
typedef struct _allocator
{
        allocate_fn _allocate;
        allocate_aligned_fn _allocate_aligned;
        deallocate_fn _deallocate;
} allocator;

static void allocator_init_ex(
        allocator* self, void* allocate, void* allocate_aligned, void* deallocate)
{
        self->_allocate = (allocate_fn)allocate;
        self->_allocate_aligned = (allocate_aligned_fn)allocate_aligned;
        self->_deallocate = (deallocate_fn)deallocate;
}

static inline void* allocate(allocator*, ssize);

static void* _allocate_aligned(void* allocator, ssize bytes, ssize alignment)
{
        void* block = allocate(allocator, bytes + alignment);
        return block
                ? align_pointer(block, alignment)
                : NULL;
}

static void allocator_init(allocator* self, void* allocate, void* deallocate)
{
        allocator_init_ex(self, allocate, &_allocate_aligned, deallocate);
}

static inline void* allocate(allocator* self, ssize bytes)
{
        S_ASSERT(self && self->_allocate);
        return self->_allocate(self, bytes);
}

static inline void* allocate_aligned(allocator* self, ssize bytes, ssize alignment)
{
        S_ASSERT(self && self->_allocate_aligned && alignment);
        return self->_allocate_aligned(self, bytes, alignment);
}

static inline void deallocate(allocator* self, void* block)
{
        S_ASSERT(self && self->_deallocate);
        self->_deallocate(self, block);
}

#endif
