#ifndef SMEMBUF_H
#define SMEMBUF_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "alloc.h"
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
extern serrcode membuf_resize(membuf* self, ssize size);

// doubles the size of membuf
extern serrcode membuf_grow(membuf* self);
// new size = old size * mul + cst
extern serrcode membuf_resize_ex(membuf* self, ssize mul, ssize cst);
// doesnt copy elements
extern void membuf_move(membuf* to, membuf* from);

static inline ssize membuf_size(const membuf* self)
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

#endif // !SMEMBUF_H
