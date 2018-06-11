#ifndef SCC_CORE_ALLOCATOR_H
#define SCC_CORE_ALLOCATOR_H

#include "common.h"
#include "misc.h"

typedef void*(*allocate_fn)(void*, size_t);
typedef void*(*allocate_aligned_fn)(void*, size_t, size_t);
typedef void(*deallocate_fn)(void*, void*);

// an allocator for 'heavy' allocations that is used by various data structures
typedef struct _allocator
{
        allocate_fn alloc;
        allocate_aligned_fn aalloc;
        deallocate_fn dealloc;
} allocator;

static void allocator_init_ex(
        allocator* self, void* allocate, void* allocate_aligned, void* deallocate)
{
        self->alloc = (allocate_fn)allocate;
        self->aalloc = (allocate_aligned_fn)allocate_aligned;
        self->dealloc = (deallocate_fn)deallocate;
}

static inline void* allocate(allocator*, size_t);

static void* _allocate_aligned(void* allocator, size_t bytes, size_t alignment)
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

static inline void* allocate(allocator* self, size_t bytes)
{
        assert(self && self->alloc);
        return self->alloc(self, bytes);
}

static inline void* allocate_aligned(allocator* self, size_t bytes, size_t alignment)
{
        assert(self && self->aalloc && alignment);
        return self->aalloc(self, bytes, alignment);
}

static inline void deallocate(allocator* self, void* block)
{
        assert(self && self->dealloc);
        self->dealloc(self, block);
}

#endif
