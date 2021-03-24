#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "list.h"

#include <stddef.h>

static inline void* align_pointer(void* p, unsigned alignment)
{
        return (void*)(((size_t)p + alignment - 1) & ~(size_t)(alignment - 1));
}

static inline unsigned pointer_adjustment(void* p, unsigned alignment)
{
        return (unsigned)((char*)align_pointer(p, alignment) - (char*)p);
}

struct stack_alloc
{
        char* chunk_pos;
        char* chunk_end;
        struct list chunks;
};

void init_stack_alloc(struct stack_alloc* self);
void drop_stack_alloc(struct stack_alloc* self);
void stack_alloc_grow(struct stack_alloc* self, size_t at_least);

static void* stack_alloc_aligned(struct stack_alloc* self, size_t size, unsigned alignment)
{
        unsigned adjustment = pointer_adjustment(self->chunk_pos, alignment);
        if (self->chunk_pos + size + adjustment >= self->chunk_end) {
                stack_alloc_grow(self, size + adjustment);
                adjustment = pointer_adjustment(self->chunk_pos, alignment);
        }
        void* block = self->chunk_pos + adjustment;
        self->chunk_pos += size + adjustment;
        return block;
}

static void* stack_alloc(struct stack_alloc* self, size_t size)
{
        return stack_alloc_aligned(self, size, sizeof(void*));
}

struct object_pool
{
        struct stack_alloc stack;
        void* top;
        unsigned object_size;
};

void init_object_pool(struct object_pool* self, size_t object_size);
void drop_object_pool(struct object_pool* self);

static void* object_pool_alloc(struct object_pool* self)
{
        if (!self->top)
                self->top = stack_alloc(&self->stack, (sizeof(void*) + self->object_size) * 2048);
        void* object = (char*)self->top + sizeof(void*);
        self->top = *(void**)self->top;
        return object;
}

static void object_pool_dealloc(struct object_pool* self, void* object)
{
        object = (char*)object - sizeof(void*);
        *(void**)object = self->top;
        self->top = object;
}

#endif
