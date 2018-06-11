// before including this header the following macro should be defined:
//      HTAB_FN(NAME) - hash table function generator
//      HTAB_TP - hash table type
//      HTAB_ETP - hash table entry type
//      HTAB_KTP - key type
//      HTAB_EK - empty key
//      HTAB_DK - deleted key
//      HTAB_VTP - optional value type

#include "memory.h"
#include <string.h> // memcpy

#ifndef HTAB_FN
#error HTAB_FN undefined
#endif

#ifndef HTAB_TP
#error HTAB_TP undefined
#endif

#ifndef HTAB_ETP
#error HTAB_ETP undefined
#endif

#ifndef HTAB_KTP
#error HTAB_KTP undefined
#endif

#ifndef HTAB_EK
#error HTAB_EK undefined
#endif

#ifndef HTAB_DK
#error HTAB_DK undefined
#endif

typedef struct
{
        HTAB_KTP key;
#ifdef HTAB_VTP
        HTAB_VTP value;
#endif
} HTAB_ETP;

typedef struct
{
        HTAB_ETP* entries;
        size_t num_entries;
        size_t size;
        allocator* alloc;
} HTAB_TP;

#ifndef HTAB_FOREACH
#define HTAB_FOREACH(PTAB, ETP, IT, FIRST_ENTRY, NEXT_ENTRY) \
        for (ETP* IT = FIRST_ENTRY(PTAB); IT; IT = NEXT_ENTRY(PTAB, IT))
#endif

static void HTAB_FN(init_ex)(HTAB_TP* self, allocator* alloc)
{
        self->alloc = alloc;
        self->entries = NULL;
        self->num_entries = 0;
        self->size = 0;
}

static void HTAB_FN(init)(HTAB_TP* self)
{
        HTAB_FN(init_ex)(self, STDALLOC);
}

static HTAB_TP HTAB_FN(create_ex)(allocator* alloc)
{
        HTAB_TP v;
        HTAB_FN(init_ex)(&v, alloc);
        return v;
}

static HTAB_TP HTAB_FN(create)()
{
        return HTAB_FN(create_ex)(STDALLOC);
}

static void HTAB_FN(dispose)(HTAB_TP* self)
{
        deallocate(self->alloc, self->entries);
        HTAB_FN(init_ex)(self, self->alloc);
}

static inline void HTAB_FN(clear)(HTAB_TP* self)
{
        for (size_t i = 0; i < self->num_entries; i++)
                self->entries[i].key = HTAB_EK;

        self->size = 0;
}

static inline HTAB_ETP* HTAB_FN(lookup_for_entry_impl)(
        const HTAB_TP* self, HTAB_KTP key, bool skip_deleted)
{
        if (!self->entries)
                return NULL;

        const size_t size = self->num_entries;

        size_t i = 1;
        size_t start = ((size_t)key) & (size - 1);
        size_t entry_no = start;

        while (1)
        {
                HTAB_ETP* entry = self->entries + entry_no;
                HTAB_KTP k = entry->key;

                if (k == key || k == HTAB_EK || (k == HTAB_DK && !skip_deleted))
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

static errcode HTAB_FN(grow)(HTAB_TP*);

static inline HTAB_ETP* HTAB_FN(insert_impl)(HTAB_TP* self, HTAB_KTP key)
{
        assert(key != HTAB_DK && key != HTAB_EK
                && "Deleted/Empty keys should not be used.");

        size_t critical = self->num_entries >> 1;
        if (self->size >= critical)
        {
                bool has_free_entries = self->size != self->num_entries;
                if (EC_FAILED(HTAB_FN(grow)(self)) && !has_free_entries)
                        return NULL;
        }

        return HTAB_FN(lookup_for_entry_impl)(self, key, false);
}

static inline HTAB_ETP* HTAB_FN(update_impl)(HTAB_TP* self, HTAB_KTP key)
{
        HTAB_ETP* entry = HTAB_FN(insert_impl)(self, key);
        if (!entry)
                return NULL;

        if (entry->key == HTAB_EK)
                self->size++;

        entry->key = key;
        return entry;
}

static inline HTAB_ETP* HTAB_FN(lookup)(const HTAB_TP* self, HTAB_KTP key)
{
        HTAB_ETP* e = HTAB_FN(lookup_for_entry_impl)(self, key, true);
        return e && e->key == key ? e : NULL;
}

#ifdef HTAB_VTP

static inline errcode HTAB_FN(insert_p)(HTAB_TP* self, HTAB_KTP key, HTAB_VTP const* val)
{
        HTAB_ETP* entry = HTAB_FN(insert_impl)(self, key);
        if (!entry)
                return EC_ERROR;

        if (entry->key == HTAB_EK)
                self->size++;
        else if (entry->key != HTAB_DK)
                return EC_NO_ERROR; 

        entry->value = *val;
        entry->key = key;
        return EC_NO_ERROR;
}

static inline errcode HTAB_FN(insert)(HTAB_TP* self, HTAB_KTP key, HTAB_VTP val)
{
        return HTAB_FN(insert_p)(self, key, &val);
}

static inline errcode HTAB_FN(update_p)(HTAB_TP* self, HTAB_KTP key, HTAB_VTP const* val)
{
        HTAB_ETP* entry = HTAB_FN(update_impl)(self, key);
        if (!entry)
                return EC_ERROR;

        entry->value = *val;
        return EC_NO_ERROR;
}

static inline errcode HTAB_FN(update)(HTAB_TP* self, HTAB_KTP key, HTAB_VTP val)
{
        return HTAB_FN(update_p)(self, key, &val);
}

#else

static inline errcode HTAB_FN(insert)(HTAB_TP* self, HTAB_KTP key)
{
        return HTAB_FN(update_impl)(self, key) ? EC_NO_ERROR : EC_ERROR;
}

#endif

static inline bool HTAB_FN(has)(const HTAB_TP* self, HTAB_KTP key)
{
        return HTAB_FN(lookup)(self, key) != NULL;
}

static inline bool HTAB_FN(erase)(HTAB_TP* self, HTAB_KTP key)
{
        HTAB_ETP* entry = HTAB_FN(lookup)(self, key);
        if (!entry)
                return false;

        entry->key = HTAB_DK;
        self->size--;
        return true;
}

static inline errcode HTAB_FN(resize_impl)(HTAB_TP* self, size_t new_size)
{
        assert(IS_POWEROF2(new_size));
        HTAB_ETP* new_entries = allocate(self->alloc, new_size * sizeof(HTAB_ETP));
        if (!new_entries)
                return EC_ERROR;

        HTAB_TP new_tab;
        new_tab.entries = new_entries;
        new_tab.num_entries = new_size;
        new_tab.size = 0;
        new_tab.alloc = self->alloc;
        HTAB_FN(clear)(&new_tab);

        for (size_t i = 0; i < self->num_entries; i++)
        {
                HTAB_ETP* entry = self->entries + i;
                HTAB_KTP key = entry->key;
                if (key == HTAB_EK || key == HTAB_DK)
                        continue;

#ifdef HTAB_VTP
                HTAB_FN(insert_p)(&new_tab, key, &entry->value);
#else
                HTAB_FN(insert)(&new_tab, key);
#endif
        }

        HTAB_FN(dispose)(self);
        *self = new_tab;

        return EC_NO_ERROR;
}

static inline errcode HTAB_FN(grow)(HTAB_TP* self)
{
        return HTAB_FN(resize_impl)(self, self->num_entries ? self->num_entries << 1 : 2);
}

static inline errcode HTAB_FN(reserve)(HTAB_TP* self, const size_t at_least)
{
        if (at_least <= self->num_entries - self->size)
                return EC_NO_ERROR;

        size_t new_size = self->num_entries;
        while (at_least > new_size - self->size)
                new_size *= 2;

        return HTAB_FN(resize_impl)(self, new_size);
}

static inline HTAB_ETP* HTAB_FN(next_entry)(const HTAB_TP* tab, HTAB_ETP* pos)
{
        const HTAB_ETP* end = tab->entries + tab->num_entries;
        assert(pos && pos < end);

        HTAB_ETP* next = pos + 1;
        while (next != end)
        {
                if (next->key != HTAB_EK && next->key != HTAB_DK)
                        return next;
                next++;
        }
        return NULL;
}

static inline HTAB_ETP* HTAB_FN(first_entry)(const HTAB_TP* tab)
{
        HTAB_ETP* e = tab->entries;
        return e && (e->key == HTAB_EK || e->key == HTAB_DK)
                ? HTAB_FN(next_entry)(tab, e)
                : e;
}

#undef HTAB_FN
#undef HTAB_TP
#undef HTAB_KTP
#undef HTAB_EK
#undef HTAB_DK
#undef HTAB_VTP
#undef HTAB_ETP