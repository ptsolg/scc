#ifndef SDSEQ_H
#define SDSEQ_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "membuf.h"
#include <memory.h>

// a dynamic array
typedef struct _dseq
{
        membuf _base;
        ssize _size;
        ssize _obsize;
} dseq;

extern void dseq_init(dseq* self, ssize obsize);
extern void dseq_init_ex(dseq* self, ssize obsize, allocator* alloc);
extern void dseq_dispose(dseq* self);
extern void dseq_move(dseq* to, dseq* from);

extern serrcode dseq_reserve(dseq* self, ssize count);
extern serrcode dseq_resize(dseq* self, ssize new_size);

static inline membuf* dseq_base(dseq* self);
static inline const membuf* dseq_cbase(const dseq* self);
static inline allocator* dseq_alloc(const dseq* self);

static inline ssize dseq_size(const dseq* self);
static inline ssize dseq_obsize(const dseq* self);
static inline ssize dseq_available(const dseq* self);
static inline ssize dseq_total(const dseq* self);

static inline bool _dseq_maybe_grow(dseq* self);
static inline serrcode dseq_append(dseq* self, const void* object);

static inline void* dseq_last(const dseq* self);
static inline void* dseq_first(const dseq* self);

static inline void* dseq_get(const dseq* self, ssize i);
static inline void dseq_set(dseq* self, ssize i, const void* object);

static inline membuf* dseq_base(dseq* self)
{
        return &self->_base;
}

static inline const membuf* dseq_cbase(const dseq* self)
{
        return &self->_base;
}

static inline allocator* dseq_alloc(const dseq* self)
{
        return membuf_alloc(dseq_cbase(self));
}

static inline ssize dseq_size(const dseq* self)
{
        return self->_size;
}

static inline ssize dseq_obsize(const dseq* self)
{
        return self->_obsize;
}

static inline ssize dseq_available(const dseq* self)
{
        return dseq_total(self) - dseq_size(self);
}

static inline ssize dseq_total(const dseq* self)
{
        return membuf_size(dseq_cbase(self)) / dseq_obsize(self);
}

static inline bool _dseq_maybe_grow(dseq* self)
{
        return dseq_available(self)
                ? true
                : S_SUCCEEDED(dseq_reserve(self, dseq_size(self) + 5));
}

static inline serrcode dseq_append(dseq* self, const void* object)
{
        if (!_dseq_maybe_grow(self))
                return S_ERROR;

        self->_size++;
        memcpy(dseq_last(self), object, dseq_obsize(self));
        return S_NO_ERROR;
}

static inline void* dseq_last(const dseq* self)
{
        return dseq_get(self, dseq_size(self) - 1);
}

static inline void* dseq_first(const dseq* self)
{
        return dseq_get(self, 0);
}

#define DSEQ_CHECK_INDEX(SEQ, I) S_ASSERT((I) < dseq_size(SEQ) && "index is out of range")

static inline void* dseq_get(const dseq* self, ssize i)
{
        DSEQ_CHECK_INDEX(self, i);
        return (char*)membuf_begin(dseq_cbase(self)) + i * dseq_obsize(self);
}

static inline void dseq_set(dseq* self, ssize i, const void* object)
{
        DSEQ_CHECK_INDEX(self, i);
        memcpy(dseq_get(self, i), object, dseq_obsize(self));
}

#ifdef __cplusplus
}
#endif

#endif // !SDSEQ_H
