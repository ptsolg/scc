#ifndef SHTAB_H
#define SHTAB_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "membuf.h"

// a prime-sized hash table that grows almost exponentially
typedef struct _htab
{
        membuf _entries;
        int    _prime_lvl;
        ssize  _used;
        ssize  _critical;
} htab;

// note: htab assumes that NULL is empty entry, and can write non-NULL value in it

typedef uint hval;

extern void     htab_init(htab* self);
extern void     htab_init_ex(htab* self, allocator* alloc);
extern void     htab_dispose(htab* self);

// marks all buckets as empty, doesn't change size
extern void     htab_clear(htab* self);
extern serrcode htab_insert(htab* self, hval key, void* val);
extern serrcode htab_merge(htab* self, htab* other);
extern bool     htabs_are_same(const htab* a, const htab* b);

// reserves slot for an element
extern serrcode htab_reserve(htab* self, hval key);
extern bool     htab_exists(const htab* self, hval key);

// returns true if element was erased
extern bool     htab_erase(htab* self, hval key);
extern void*    htab_find(const htab* self, hval key);

static inline allocator* htab_alloc(const htab* self)
{
        return membuf_alloc(&self->_entries);
}

static inline ssize htab_size(const htab* self)
{
        return self->_used;
}

typedef void*       hiter;
typedef const void* const_hiter;

// returns first non-empty slot
extern hiter htab_begin(const htab* self);
extern hiter htab_end(const htab* self);

// moves iterator to the next non-empty slot
extern hiter hiter_get_next(hiter self, const htab* tab);
extern hval  hiter_get_key(const_hiter self);
extern void* hiter_get_val(const_hiter self);

extern void  hiter_set_key(hiter self, hval key);
extern void  hiter_set_val(hiter self, void* val);

#define HTAB_FOREACH(PTAB, ITNAME)            \
        for (hiter ITNAME = htab_begin(PTAB); \
                ITNAME != htab_end(PTAB);     \
                ITNAME = hiter_get_next(ITNAME, PTAB))

#ifdef __cplusplus
}
#endif

#endif // !SHTAB_H