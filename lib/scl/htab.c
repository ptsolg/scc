#include "scc/scl/htab.h"
#include "scc/scl/misc.h"
#include <memory.h>

#define HVAL_EMPTY ((hval)-1)

typedef struct
{
        hval key;
        char data[0];
} hentry;

extern void htab_init(htab* self, ssize obsize)
{
        htab_init_ex(self, obsize, get_std_alloc());
}

extern void htab_init_ex(htab* self, ssize obsize, allocator* alloc)
{
        S_ASSERT(obsize);
        membuf_init_ex(&self->_entries, alloc);
        self->_size = 0;
        self->_used = 0;
        self->_obsize = obsize;
}

extern void htab_dispose(htab* self)
{
        membuf_dispose(&self->_entries);
        self->_size = 0;
        self->_used = 0;
}

extern void htab_move(htab* to, htab* from)
{
        membuf_move(&to->_entries, &from->_entries);
        to->_used = from->_used;
        to->_obsize = from->_obsize;
        to->_size = from->_size;
}

static inline hval fix_hval(hval h)
{
        return h == HVAL_EMPTY ? h + 1 : h;
}

extern void htab_clear(htab* self)
{
        HTAB_FOREACH(self, it)
                ((hentry*)it._entry)->key = HVAL_EMPTY;

        self->_used = 0;
}

static inline bool htab_maybe_grow(htab* self)
{
        ssize size = self->_size;
        ssize critical = size >> 1;
        if (self->_used < critical)
                return true;

        if (!size)
                size = 2;

        ssize new_size = size << 1;
        ssize entry_size = htab_obsize(self) + sizeof(hentry);

        htab new_tab;
        htab_init_ex(&new_tab, htab_obsize(self), htab_alloc(self));
        if (S_FAILED(membuf_resize(&new_tab._entries, new_size * entry_size)))
                return false;
        new_tab._size = new_size;
        htab_clear(&new_tab);

        HTAB_FOREACH(self, it)
                htab_insert(&new_tab, hiter_get_key(&it), hiter_get_val(&it));

        htab_dispose(self);
        htab_move(self, &new_tab);
        return true;
}

static inline hiter hiter_create(hentry* entry, const htab* tab)
{
        hiter it = { (void*)entry, tab };
        return it;
}

static inline hentry* htab_find_existing_or_empty_entry(const htab* self, hval key)
{
        const ssize size = self->_size;
        const ssize entry_size = htab_obsize(self) + sizeof(hentry);
        suint8* data = (suint8*)membuf_begin(&self->_entries);
        if (!data)
                return NULL;

        ssize i = 1;
        ssize start = key & (size - 1);
        ssize entry_no = start;

        while (1)
        {
                hentry* entry = (hentry*)(data + entry_size * entry_no);
                if (entry->key == key || entry->key == HVAL_EMPTY)
                        return entry;

                entry_no += i++;
                entry_no &= (size - 1);

                if (entry_no == start)
                {
                        // if we are here, then hash table has 0 free slots
                        return NULL;
                }
        }
}

static inline hentry* htab_insert_key(htab* self, hval key)
{
        if (!htab_maybe_grow(self))
                return NULL;

        key = fix_hval(key);
        hentry* entry = htab_find_existing_or_empty_entry(self, key);
        if (!entry)
                return NULL;

        if (entry->key == HVAL_EMPTY)
                self->_used++;

        entry->key = key;
        return entry;
}

extern serrcode htab_insert(htab* self, hval key, const void* object)
{
        S_ASSERT(object);
        hentry* entry = htab_insert_key(self, key);
        if (!entry)
                return S_ERROR;

        memcpy(entry->data, object, htab_obsize(self));
        return S_NO_ERROR;
}

extern serrcode htab_merge(htab* self, const htab* other)
{
        htab result;
        htab_init_ex(&result, htab_obsize(self), htab_alloc(self));

        HTAB_FOREACH(other, it)
                if (S_FAILED(htab_insert(&result, hiter_get_key(&it), hiter_get_val(&it))))
                        goto error;
        HTAB_FOREACH(self, it)
                if (S_FAILED(htab_insert(&result, hiter_get_key(&it), hiter_get_val(&it))))
                        goto error;

        htab_dispose(self);
        htab_move(self, &result);
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
                if (htab_exists(b, hiter_get_key(&it)))
                        matches++;
        return matches == htab_size(a);
}

extern serrcode htab_reserve(htab* self, hval key)
{
        return htab_insert_key(self, key) ? S_NO_ERROR : S_ERROR;
}

extern bool htab_exists(const htab* self, hval key)
{
        hiter placeholder;
        return htab_find(self, key, &placeholder);
}

extern bool htab_erase(htab* self, hval key)
{
        S_UNREACHABLE(); // todo
        return false;
}

extern bool htab_find(const htab* self, hval key, hiter* it)
{
        key = fix_hval(key);
        hentry* entry = htab_find_existing_or_empty_entry(self, key);
        if (!entry || entry->key != key)
                return false;

        it->_entry = entry;
        it->_tab = self;
        return true;
}

extern hiter htab_begin(const htab* tab)
{
        ssize entry_size = htab_obsize(tab) + sizeof(hentry);
        hentry* it = membuf_begin(&tab->_entries);
        hentry* end = membuf_end(&tab->_entries);

        while (it != end && it->key == HVAL_EMPTY)
                it = (hentry*)((suint8*)it + entry_size);

        return hiter_create(it, tab);
}

extern bool hiter_valid(const hiter* self)
{
        return (suint8*)self->_entry != (suint8*)membuf_end(&self->_tab->_entries);
}

extern void hiter_advance(hiter* self)
{
        hentry* end = membuf_end(&self->_tab->_entries);
        hentry* it = self->_entry;
        if (it == end)
                return;

        do
                it = (hentry*)((suint8*)it + htab_obsize(self->_tab) + sizeof(hentry));
        while (it != end && it->key == HVAL_EMPTY);

        self->_entry = it;
        return;
}

extern hval hiter_get_key(const hiter* self)
{
        return ((hentry*)self->_entry)->key;
}

extern void* hiter_get_val(const hiter* self)
{
        S_ASSERT(hiter_valid(self));
        return ((hentry*)self->_entry)->data;
}

extern void hiter_set_key(const hiter* self, hval key)
{
        key = fix_hval(key);
        ((hentry*)self->_entry)->key = key;
}

extern void hiter_set_val(const hiter* self, const void* object)
{
        memcpy(hiter_get_val(self), object, htab_obsize(self->_tab));
}
