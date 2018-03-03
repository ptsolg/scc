#ifndef SCC_CORE_MEMORY_H
#define SCC_CORE_MEMORY_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h>
#include "allocator.h"
#include "list.h"
#include "error.h"

extern allocator* _get_stdalloc();

#define STDALLOC (_get_stdalloc())

extern void mallocator_init(allocator* self);

#if TARGET_X64
#define STDALIGNMENT 8
#else
#define STDALIGNMENT 4
#endif

typedef struct _obstack
{
        allocator _base;

        struct
        {
                uint8_t* pos;
                uint8_t* end;
                size_t size;
        } _chunk;

        list_head _chunks;
        allocator* _alloc;
} obstack;

extern void obstack_init(obstack* self);
extern void obstack_init_ex(obstack* self, allocator* alloc);
extern void obstack_dispose(obstack* self);
extern errcode obstack_grow(obstack* self, size_t at_least);

static inline void* obstack_allocate_aligned(obstack* self, size_t bytes, size_t alignment)
{
        size_t adjustment = pointer_adjustment(self->_chunk.pos, alignment);
        if (self->_chunk.pos + bytes + adjustment >= self->_chunk.end)
        {
                if (EC_FAILED(obstack_grow(self, bytes + alignment)))
                        return NULL;

                adjustment = pointer_adjustment(self->_chunk.pos, alignment);
        }

        void* block = self->_chunk.pos + adjustment;
        self->_chunk.pos += bytes + adjustment;
        return block;
}

static inline void* obstack_allocate(obstack* self, size_t bytes)
{
        return obstack_allocate_aligned(self, bytes, STDALIGNMENT);
}

static inline void obstack_deallocate(obstack* self, void* object)
{
        ;
}

static inline allocator* obstack_to_allocator(obstack* self)
{
        return &self->_base;
}

static inline allocator* obstack_allocator(const obstack* self)
{
        return self->_alloc;
}

typedef struct _objpool
{
        obstack _base;
        uint8_t* _top;
        size_t _obsize;
} objpool;

extern void objpool_init(objpool* self, size_t obsize);
extern void objpool_init_ex(objpool* self, size_t obsize, allocator* alloc);
extern void objpool_dispose(objpool* self);

static inline void* objpool_allocate(objpool* self)
{
        if (!self->_top)
                if (!(self->_top = obstack_allocate(&self->_base, sizeof(void*) + self->_obsize)))
                        return NULL;

        void* object = self->_top + sizeof(void*);
        self->_top = *(void**)self->_top;
        return object;
}

static inline void objpool_deallocate(objpool* self, void* object)
{
        if (!object)
                return;

        object = (uint8_t*)object - sizeof(void*);
        *(void**)object = self->_top;
        self->_top = object;
}

typedef void*(*mempool_bad_alloc_handler)(void*, size_t);

typedef struct _mempool
{
        allocator _base;
        allocator* _alloc;
        list_head _used;
        void* _on_bad_alloc;
        mempool_bad_alloc_handler _handler;
} mempool;

extern void mempool_init(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc);

extern void mempool_init_ex(
        mempool* self, mempool_bad_alloc_handler handler, jmp_buf on_bad_alloc, allocator* alloc);

extern void mempool_dispose(mempool* self);

static inline void* mempool_allocate(mempool* self, size_t bytes)
{
        void* block = allocate(self->_alloc, sizeof(list_node) + bytes);
        if (!block)
        {
                if (self->_handler)
                        block = self->_handler(self, bytes);
                if (!block)
                        longjmp(self->_on_bad_alloc, EC_ERROR);
        }

        list_node_init(block);
        list_push_back(&self->_used, block);
        return (uint8_t*)block + sizeof(list_node);
}

static inline void mempool_deallocate(mempool* self, void* block)
{
        if (!block)
                return;

        block = (uint8_t*)block - sizeof(list_node);
        list_node_remove(block);
        deallocate(self->_alloc, block);
}

static inline allocator* mempool_to_allocator(mempool* self)
{
        return &self->_base;
}

#ifdef __cplusplus
}
#endif

#endif