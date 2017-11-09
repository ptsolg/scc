#include "scc/scl/htab.h"

static const ssize primes[] =
{
        7,
        13,
        31,
        61,
        127,
        251,
        509,
        1021,
        2039,
        4093,
        8191,
        16381,
        32749,
        65521,
        131071,
        262139,
        524287,
        1048573,
        2097143,
        4194301,
        8388593,
        16777213,
        33554393,
        67108859,
        134217689,
        268435399,
        536870909,
        1073741789,
        2147483647,
};

typedef struct
{
        void* val;
        hval key;
} hentry;

extern void htab_init(htab* self)
{
        htab_init_ex(self, get_std_alloc());
}

extern void htab_init_ex(htab* self, allocator* alloc)
{
        membuf_init_ex(&self->_entries, alloc);
        self->_used = 0;
        self->_critical = 0; 
        self->_prime_lvl = 0;
}

extern void htab_dispose(htab* self)
{
        membuf_dispose(&self->_entries);
        self->_used = 0;
        self->_critical = 0;
        self->_prime_lvl = 0;
}

extern void htab_clear(htab* self)
{
        if (!membuf_size(&self->_entries))
                return;

        HTAB_FOREACH(self, it)
                hiter_set_val(it, NULL);

        self->_used = 0;
}

static inline serrcode htab_grow(htab* self)
{
        ssize new_size = primes[self->_prime_lvl + 1];
        membuf new_buf;
        membuf old_buf = self->_entries;

        membuf_init_ex(&new_buf, membuf_alloc(&old_buf));
        if (S_FAILED(membuf_resize(&new_buf, new_size * sizeof(hentry))))
                return S_ERROR;
        
        self->_prime_lvl++;
        self->_critical = (ssize)(new_size * 0.5) + 1;
        self->_entries = new_buf;
        htab_clear(self);

        MEMBUF_FOREACH(&old_buf, hentry*, entry)
                if (entry->val)
                        htab_insert(self, entry->key, entry->val);

        membuf_dispose(&old_buf);
        return S_NO_ERROR;
}

static inline hiter htab_find_existing_or_empty_entry(const htab* self, hval key)
{
        hiter begin = membuf_begin(&self->_entries);
        hiter end = membuf_end(&self->_entries);
        hiter sep = (hentry*)begin + key % primes[self->_prime_lvl];

        for (hiter it = sep; it != end; it = (hentry*)it + 1)
                if (!hiter_get_val(it) || hiter_get_key(it) == key)
                        return it;

        for (hiter it = begin; it != sep; it = (hentry*)it + 1)
                if (!hiter_get_val(it) || hiter_get_key(it) == key)
                        return it;

        // if we are here, then hash table has 0 free slots which is imposible
        S_UNREACHABLE();
        return NULL;
}

extern serrcode htab_insert(htab* self, hval key, void* val)
{
        if (self->_used >= self->_critical)
                if (S_FAILED(htab_grow(self)))
                        return S_ERROR;

        hiter it = htab_find_existing_or_empty_entry(self, key);
        if (!hiter_get_val(it))
                self->_used++;

        hiter_set_val(it, val);
        hiter_set_key(it, key);
        return S_NO_ERROR;
}

extern serrcode htab_merge(htab* self, htab* other)
{
        htab result;
        htab_init_ex(&result, htab_alloc(self));

        HTAB_FOREACH(other, it)
                if (S_FAILED(htab_insert(&result, hiter_get_key(it), hiter_get_val(it))))
                        goto error;
        HTAB_FOREACH(self, it)
                if (S_FAILED(htab_insert(&result, hiter_get_key(it), hiter_get_val(it))))
                        goto error;

        htab_dispose(self);
        *self = result;
        return S_NO_ERROR;

error:
        htab_dispose(&result);
        return S_ERROR;
}

extern bool htabs_are_same(const htab* a, const htab* b)
{
        if (htab_size(a) != htab_size(b))
                return false;

        ssize matches = 0;
        HTAB_FOREACH(a, it)
                if (htab_exists(b, hiter_get_key(it)))
                        matches++;
        return matches == htab_size(a);
}

extern serrcode htab_reserve(htab* self, hval key)
{
        return htab_insert(self, key, (void*)-1);
}

extern bool htab_exists(const htab* self, hval key)
{
        return htab_find(self, key) != NULL;
}

extern bool htab_erase(htab* self, hval key)
{
        S_UNREACHABLE();// todo
        return false;
}

extern void* htab_find(const htab* self, hval key)
{
        if (!membuf_size(&self->_entries))
                return NULL;

        hentry* e = htab_find_existing_or_empty_entry(self, key);
        return e->val ? e->val : NULL;
}

extern hiter htab_begin(const htab* self)
{
        hiter begin = membuf_begin(&self->_entries);
        hiter end = htab_end(self);

        for (hiter it = begin; it != end; it = (hentry*)it + 1)
                if (hiter_get_val(it))
                        return it;

        return end;
}

extern hiter htab_end(const htab* self)
{
        return (hiter)membuf_end(&self->_entries);
}

extern hiter hiter_get_next(hiter self, const htab* tab)
{
        hiter next = (hentry*)self + 1;
        hiter end = htab_end(tab);
        for (hiter it = next; it != end; it = (hentry*)it + 1)
                if (hiter_get_val(it))
                        return it;
        return end;
}

extern hval hiter_get_key(const_hiter self)
{
        return ((const hentry*)self)->key;
}

extern void* hiter_get_val(const_hiter self)
{
        return ((const hentry*)self)->val;
}

extern void hiter_set_key(hiter self, hval key)
{
        ((hentry*)self)->key = key;
}

extern void hiter_set_val(hiter self, void* val)
{
        ((hentry*)self)->val = val;
}