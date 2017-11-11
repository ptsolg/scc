#ifndef SALLOC_H
#define SALLOC_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "list.h"
#include "error.h"
#include "misc.h"

typedef void*(*alloc_fn)(void*, ssize);
typedef void*(*alloc_ex_fn)(void*, ssize, ssize);
typedef void*(*dealloc_fn)(void*, void*);

// an allocator for 'heavy' allocations that is used by various data structures
typedef struct _allocator
{
        alloc_fn _allocate;
        alloc_ex_fn _allocate_ex;
        dealloc_fn _deallocate;
} allocator;

static inline void allocator_init_ex(
        allocator* self, void* alloc, void* alloc_ex, void* dealloc)
{
        self->_allocate = (alloc_fn)alloc;
        self->_allocate_ex = (alloc_ex_fn)alloc_ex;
        self->_deallocate = (dealloc_fn)dealloc;
}

static inline void allocator_init(
        allocator* self, void* alloc, void* dealloc)
{
        allocator_init_ex(self, alloc, NULL, dealloc);
}

static inline void* allocate(allocator* self, ssize bytes)
{ 
        S_ASSERT(self && self->_allocate);
        return self->_allocate(self, bytes); 
}

static inline void* allocate_ex(allocator* self, ssize bytes, ssize align)
{
        S_ASSERT(self && self->_allocate_ex);
        return self->_allocate_ex(self, bytes, align);
}

static inline void deallocate(allocator* self, void* block)
{ 
        S_ASSERT(self && self->_deallocate);
        self->_deallocate(self, block);
}

#if S_X64
#define BPA_ALIGN 8
#else
#define BPA_ALIGN 4
#endif

typedef struct
{
        list_node _node;
        ssize _size;
        char _bytes[0];
} _bp_chunk;

typedef struct _bp_allocator
{
        allocator _base;
        char* _pos;
        char* _end;
        list_head _chunks;
        allocator* _alloc;
} bp_allocator;

extern void bpa_init(bp_allocator* self);
extern void bpa_init_ex(bp_allocator* self, allocator* alloc);
extern void bpa_dispose(bp_allocator* self);

static inline allocator* bpa_base(bp_allocator* self)
{
        return &self->_base;
}

static inline const allocator* bpa_cbase(const bp_allocator* self)
{
        return &self->_base;
}

static inline bool bpa_can_allocate(const bp_allocator* self, ssize bytes, ssize align)
{
        ssize adjustment = pointer_adjustment(self->_pos, align);
        return self->_pos + adjustment < self->_end;
}

extern _bp_chunk* _bpa_new_chunk(bp_allocator* self, ssize bytes);
extern void _bpa_use_chunk(bp_allocator* self, _bp_chunk* c);
extern serrcode _bpa_grow(bp_allocator* self, ssize cst);

static inline void* bp_allocate_ex(bp_allocator* self, ssize bytes, ssize alignment)
{
        ssize adjustment = pointer_adjustment(self->_pos, alignment);
        if (self->_pos + bytes + adjustment >= self->_end)
        {
                if (S_FAILED(_bpa_grow(self, bytes + alignment)))
                        return NULL;
                adjustment = pointer_adjustment(self->_pos, alignment);
        }

        void* result = self->_pos + adjustment;
        self->_pos += bytes + adjustment;
        return result;
}

static inline void* bp_allocate(bp_allocator* self, ssize bytes)
{
        return bp_allocate_ex(self, bytes, BPA_ALIGN);
}

static inline allocator* bpa_alloc(const bp_allocator* self)
{
        return self->_alloc;
}

typedef struct
{
        void* _next;
        char _bytes[0];
} _obj_header;

// an allocator for objects with the same size
typedef struct _obj_allocator
{
        bp_allocator _base;
        void* _top;
        ssize _objects;
        ssize _obsize;
} obj_allocator;

extern void obj_allocator_init(obj_allocator* self, ssize obsize);
extern void obj_allocator_init_ex(obj_allocator* self, ssize obsize, allocator* alloc);
extern serrcode _obj_allocator_grow(obj_allocator* self);

static inline bp_allocator* obj_allocator_base(obj_allocator* self)
{
        return &self->_base;
}

static inline const bp_allocator* obj_allocator_cbase(const obj_allocator* self)
{
        return &self->_base;
}

static inline void obj_deallocate(obj_allocator* self, void* obj)
{
        if (!obj)
                return;

        _obj_header* h = (_obj_header*)obj;
        h->_next = self->_top;
        self->_top = h;
}

static inline void* obj_allocate(obj_allocator* self)
{
        if (!self->_top)
                if (S_FAILED(_obj_allocator_grow(self)))
                        return NULL;

        S_ASSERT(self->_top);
        _obj_header* top = (_obj_header*)self->_top;
        self->_top = top->_next;
        return top->_bytes;
}

extern allocator* get_std_alloc();

#ifdef __cplusplus
}
#endif

#endif // !SALLOC_H
