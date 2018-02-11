#ifndef SMEMORY_H
#define SMEMORY_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <setjmp.h>
#include "allocator.h"
#include "list.h"
#include "error.h"

#if S_X64
#define STDALIGNMENT 8
#else
#define STDALIGNMENT 4
#endif

typedef struct _obstack
{
        allocator _base;

        struct
        {
                suint8* pos;
                suint8* end;
                ssize size;
        } _chunk;

        list_head _chunks;
        allocator* _alloc;
} obstack;

extern void obstack_init(obstack* self);
extern void obstack_init_ex(obstack* self, allocator* alloc);
extern void obstack_dispose(obstack* self);
extern serrcode obstack_grow(obstack* self, ssize at_least);

static inline void* obstack_allocate_aligned(obstack* self, ssize bytes, ssize alignment)
{
        ssize adjustment = pointer_adjustment(self->_chunk.pos, alignment);
        if (self->_chunk.pos + bytes + adjustment >= self->_chunk.end)
        {
                if (!obstack_grow(self, bytes + alignment))
                        return NULL;

                adjustment = pointer_adjustment(self->_chunk.pos, alignment);
        }

        void* block = self->_chunk.pos + adjustment;
        self->_chunk.pos += bytes + adjustment;
        return block;
}

static inline void* obstack_allocate(obstack* self, ssize bytes)
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

typedef struct _object_allocator
{
        obstack _base;
        suint8* _top;
        ssize _obsize;
} object_allocator;

extern void object_allocator_init(object_allocator* self, ssize obsize);
extern void object_allocator_init_ex(object_allocator* self, ssize obsize, allocator* alloc);
extern void object_allocator_dispose(object_allocator* self);

static inline void* object_allocate(object_allocator* self);
static inline void object_deallocate(object_allocator* self, void* object);

typedef void*(*bad_alloc_handler)(void*, ssize);

typedef struct _base_allocator
{
        allocator _base;
        allocator* _alloc;
        list_head _used;
        void* _on_bad_alloc;
        bad_alloc_handler _handler;
} base_allocator;

extern void base_allocator_init(
        base_allocator* self, bad_alloc_handler handler, void* on_bad_alloc);

extern void base_allocator_init_ex(
        base_allocator* self, bad_alloc_handler handler, void* on_bad_alloc, allocator* alloc);

extern void base_allocator_dispose(base_allocator* self);

static inline void* base_allocate(base_allocator* self, ssize bytes);
static inline void base_deallocate(base_allocator* self, void* block);

static inline allocator* base_allocator_base(base_allocator* self);

typedef struct _malloc_allocator
{
        allocator _base;
} malloc_allocator;

extern void malloc_allocator_init(malloc_allocator* self);
extern void malloc_allocator_dispose(malloc_allocator* self);

extern void* mallocate(malloc_allocator* self, ssize bytes);
extern void mdeallocate(malloc_allocator* self, void* block);

static inline allocator* malloc_allocator_base(malloc_allocator* self);

extern allocator* _get_stdalloc();

#define STDALLOC (_get_stdalloc())

static inline void* object_allocate(object_allocator* self)
{
        if (!self->_top)
                if (!(self->_top = obstack_allocate(&self->_base, sizeof(void*) + self->_obsize)))
                        return NULL;

        void* object = self->_top + sizeof(void*);
        self->_top = *(void**)self->_top;
        return object;
}

static inline void object_deallocate(object_allocator* self, void* object)
{
        if (!object)
                return;

        object = (suint8*)object - sizeof(void*);
        *(void**)object = self->_top;
        self->_top = object;
}

static inline void* base_allocate(base_allocator* self, ssize bytes)
{
        void* block = allocate(self->_alloc, sizeof(list_node) + bytes);
        if (!block)
        {
                if (self->_handler)
                        block = self->_handler(self, bytes);
                if (!block)
                        longjmp(self->_on_bad_alloc, S_ERROR);
        }

        list_node_init(block);
        list_push_back(&self->_used, block);
        return (suint8*)block + sizeof(list_node);
}

static inline void base_deallocate(base_allocator* self, void* block)
{
        if (!block)
                return;

        block = (suint8*)block - sizeof(list_node);
        list_node_remove(block);
        deallocate(self->_alloc, block);
}

static inline allocator* base_allocator_base(base_allocator* self)
{
        return &self->_base;
}

static inline allocator* malloc_allocator_base(malloc_allocator* self)
{
        return &self->_base;
}

#ifdef __cplusplus
}
#endif

#endif