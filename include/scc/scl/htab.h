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
        ssize _used;
        ssize _obsize;
        ssize _size;
} htab;

typedef struct _hiter
{
        void* _entry;
        const htab* _tab;
} hiter;

typedef uint hval;

extern void htab_init(htab* self, ssize obsize);
extern void htab_init_ex(htab* self, ssize obsize, allocator* alloc);
extern void htab_dispose(htab* self);
extern void htab_move(htab* to, htab* from);

// marks all buckets as empty, doesn't change size
extern void htab_clear(htab* self);
extern serrcode htab_insert(htab* self, hval key, const void* object);
extern serrcode htab_merge(htab* self, const htab* other);
extern bool htabs_are_same(const htab* a, const htab* b);

// reserves slot for an element
extern serrcode htab_reserve(htab* self, hval key);
extern bool htab_exists(const htab* self, hval key);

// returns true if element was erased
extern bool htab_erase(htab* self, hval key);
// returns true if element was found
extern bool htab_find(const htab* self, hval key, hiter* it);

static inline allocator* htab_alloc(const htab* self)
{
        return membuf_alloc(&self->_entries);
}

static inline ssize htab_size(const htab* self)
{
        return self->_used;
}

static inline ssize htab_obsize(const htab* self)
{
        return self->_obsize;
}

extern hiter htab_begin(const htab* tab);

extern bool hiter_valid(const hiter* self);
extern void hiter_advance(hiter* self);

extern hval hiter_get_key(const hiter* self);
extern void* hiter_get_val(const hiter* self);

extern void hiter_set_key(const hiter* self, hval key);
extern void hiter_set_val(const hiter* self, const void* object);

#define HTAB_FOREACH(PTAP, ITNAME) \
        for (hiter ITNAME = htab_begin(PTAP); hiter_valid(&ITNAME); hiter_advance(&ITNAME))

#ifdef __cplusplus
}
#endif

#endif // !SHTAB_H
