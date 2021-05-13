#include <assert.h>

#include "alloc.h"

void* memcpy(void*, const void*, size_t);

#ifndef HTAB_K
#error HTAB_K undefined
#endif

#ifndef HTAB_K_EMPTY
#error HTAB_K_EMPTY undefined
#endif

#ifndef HTAB_K_DEL
#error HTAB_K_DEL undefined
#endif

#ifndef HTAB
#error HTAB undefined
#endif

#ifndef HTAB_K_TO_U32
#define HTAB_K_TO_U32(K) ((unsigned)K)
#endif

#ifndef HTAB_EQ
#define HTAB_EQ(K1, K2) ((K1) == (K2))
#endif

#define HTAB_NEQ(K1, K2) !(HTAB_EQ(K1, K2))

#define HTAB__CONCAT(A, B) A ## B
#define HTAB_CONCAT(A, B) HTAB__CONCAT(A, B)

#ifndef HTAB_ENTRY
#define HTAB_ENTRY HTAB_CONCAT(HTAB, _entry)
#endif

#ifndef HTAB_F
#define HTAB_F(x) HTAB_CONCAT(HTAB_CONCAT(HTAB, _), x)
#endif

#ifndef HTAB_ITER
#define HTAB_ITER HTAB_CONCAT(HTAB, _iter)
#endif

struct HTAB_ENTRY
{
        HTAB_K key;
#ifdef HTAB_V
        HTAB_V value;
#endif
};

struct HTAB
{
        struct HTAB_ENTRY* entries;
#ifdef __SCC__
        unsigned size;
        unsigned epoch;
#else
        unsigned size : 27;
        unsigned char epoch : 5;
#endif
};

struct HTAB_ITER
{
        struct HTAB_ENTRY* pos;
        struct HTAB_ENTRY* end;
};

static void HTAB_F(init)(struct HTAB* self)
{
        self->entries = 0;
        self->size = 0;
        self->epoch = 0;
}

static struct HTAB* HTAB_F(new)(void)
{
        struct HTAB* h = alloc(sizeof(struct HTAB));
        HTAB_F(init)(h);
        return h;
}

static void HTAB_F(drop)(struct HTAB* self)
{
        dealloc(self->entries);
        HTAB_F(init)(self);
}

static void HTAB_F(del)(struct HTAB* self)
{
        HTAB_F(drop)(self);
        dealloc(self);
}

static inline unsigned HTAB_F(num_entries)(const struct HTAB* self)
{
        return self->epoch ? (unsigned)1 << self->epoch : 0;
}

static void HTAB_F(clear)(struct HTAB* self)
{
        for (struct HTAB_ENTRY* it = self->entries;
             it != self->entries + HTAB_F(num_entries)(self); it++) {
                it->key = HTAB_K_EMPTY;
        }
        self->size = 0;
}

static inline struct HTAB_ENTRY*
HTAB_F(lookup_for_entry_impl)(const struct HTAB* self, HTAB_K key, int skip_deleted)
{
        assert(HTAB_NEQ(key, HTAB_K_DEL)
               && HTAB_NEQ(key, HTAB_K_EMPTY)
               && "Deleted/Empty keys should not be used.");

        if (!self->entries)
                return 0;

        const unsigned size = HTAB_F(num_entries)(self);

        unsigned i = 1;
        unsigned start = HTAB_K_TO_U32(key) & (size - 1);
        unsigned entry_no = start;

        while (1) {
                struct HTAB_ENTRY* entry = self->entries + entry_no;
                HTAB_K k = entry->key;

                if (HTAB_EQ(k, key)
                    || HTAB_EQ(k, HTAB_K_EMPTY)
                    || (HTAB_EQ(k, HTAB_K_DEL) && !skip_deleted)) {
                        return entry;
                }

                entry_no += i++;
                entry_no &= (size - 1);

                if (entry_no == start) {
                        assert(0 && "Hash table has 0 free entries.");
                        return 0;
                }
        }
}

static void HTAB_F(grow)(struct HTAB*);

static inline struct HTAB_ENTRY*
HTAB_F(insert_impl)(struct HTAB* self, HTAB_K key)
{
        size_t num_entries = HTAB_F(num_entries)(self);
        size_t critical = num_entries >> 1;
        if (self->size >= critical)
                HTAB_F(grow)(self);
        return HTAB_F(lookup_for_entry_impl)(self, key, 0);
}

static inline struct HTAB_ENTRY*
HTAB_F(update_impl)(struct HTAB* self, HTAB_K key)
{
        struct HTAB_ENTRY* entry = HTAB_F(insert_impl)(self, key);
        if (HTAB_EQ(entry->key, HTAB_K_EMPTY))
                self->size++;
        entry->key = key;
        return entry;
}

static inline struct HTAB_ENTRY*
HTAB_F(lookup)(const struct HTAB* self, HTAB_K key)
{
        struct HTAB_ENTRY* e = HTAB_F(lookup_for_entry_impl)(self, key, 1);
        return e && HTAB_EQ(e->key, key) ? e : 0;
}

#ifdef HTAB_V

static inline void HTAB_F(insert)(struct HTAB* self, HTAB_K key, HTAB_V val)
{
        struct HTAB_ENTRY* entry = HTAB_F(insert_impl)(self, key);
        if (HTAB_EQ(entry->key, HTAB_K_EMPTY))
                self->size++;
        else if (HTAB_NEQ(entry->key, HTAB_K_DEL))
                return;

        entry->value = val;
        entry->key = key;
}

static inline void HTAB_F(update)(struct HTAB* self, HTAB_K key, HTAB_V val)
{
        struct HTAB_ENTRY* entry = HTAB_F(update_impl)(self, key);
        entry->value = val;
}

#else

static inline void HTAB_F(insert)(struct HTAB* self, HTAB_K key)
{
        HTAB_F(update_impl)(self, key);
}

#endif

static inline int HTAB_F(has)(const struct HTAB* self, HTAB_K key)
{
        return HTAB_F(lookup)(self, key) != 0;
}

static inline int HTAB_F(erase)(struct HTAB* self, HTAB_K key)
{
        struct HTAB_ENTRY* entry = HTAB_F(lookup)(self, key);
        if (!entry)
                return 0;

        entry->key = HTAB_K_DEL;
        self->size--;
        return 1;
}

static inline int HTAB_F(key_ok)(HTAB_K key)
{
        return HTAB_NEQ(key, HTAB_K_EMPTY) && HTAB_NEQ(key, HTAB_K_DEL);
}

static inline void HTAB_F(grow)(struct HTAB* self)
{
        struct HTAB new_tab;
        new_tab.epoch = self->epoch + 1;
        new_tab.size = 0;
        new_tab.entries = alloc(HTAB_F(num_entries)(&new_tab) * sizeof(struct HTAB_ENTRY));
        HTAB_F(clear)(&new_tab);

        struct HTAB_ENTRY* entries_end = self->entries + HTAB_F(num_entries)(self);
        for (struct HTAB_ENTRY* it = self->entries; it != entries_end; it++) {
                if (!HTAB_F(key_ok)(it->key))
                        continue;
#ifdef HTAB_V
                HTAB_F(insert)(&new_tab, it->key, it->value);
#else
                HTAB_F(insert)(&new_tab, it->key);
#endif
        }

        HTAB_F(drop)(self);
        *self = new_tab;
}

static inline void HTAB_F(next)(struct HTAB_ITER* it)
{
        if (it->pos != it->end)
                it->pos++;
        while (it->pos != it->end && !HTAB_F(key_ok)(it->pos->key))
                it->pos++;
}

static struct HTAB_ITER HTAB_F(begin)(const struct HTAB* self)
{
        struct HTAB_ITER it = {
                self->entries,
                self->entries + HTAB_F(num_entries)(self)
        };
        if (!self->size)
                it.pos = it.end;
        else if (!HTAB_F(key_ok)(it.pos->key))
                HTAB_F(next)(&it);
        return it;
}

#undef HTAB_ITER
#undef HTAB_F
#undef HTAB_ENTRY
#undef HTAB__CONCAT
#undef HTAB_CONCAT
#undef HTAB_NEQ
#undef HTAB_EQ
#undef HTAB_K_TO_U32
#undef HTAB
#undef HTAB_K_DEL
#undef HTAB_K_EMPTY
#undef HTAB_K
#undef HTAB_V
