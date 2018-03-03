#ifndef SCC_CORE_MEMBUF_H
#define SCC_CORE_MEMBUF_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "allocator.h"
#include "error.h"
#include "misc.h"

// struct used for dynamic memory allocation management
typedef struct
{
        uchar* _begin;
        uchar* _end;
        allocator* _alloc;
} membuf;

extern void membuf_init(membuf* self);
extern void membuf_init_ex(membuf* self, allocator* alloc);
extern void membuf_dispose(membuf* self);
extern errcode membuf_resize(membuf* self, size_t size);

// doubles the size of membuf
extern errcode membuf_grow(membuf* self);
// new size = old size * mul + cst
extern errcode membuf_resize_ex(membuf* self, size_t mul, size_t cst);
// doesnt copy elements
extern void membuf_move(membuf* to, membuf* from);

static inline size_t membuf_size(const membuf* self)
{
        return self->_end - self->_begin;
}

static inline void* membuf_begin(const membuf* self)
{
        return self->_begin;
}

static inline void* membuf_end(const membuf* self)
{
        return self->_end;
}

static inline allocator* membuf_alloc(const membuf* self)
{
        return self->_alloc;
}

#define MEMBUF_FOREACH(PBUF, ITTYPE, ITNAME) \
        MEM_FOREACH((PBUF)->_begin, (PBUF)->_end, ITTYPE, ITNAME)

#ifdef __cplusplus
}
#endif

#endif
