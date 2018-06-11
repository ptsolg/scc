#ifndef _TM_IMPL_H
#define _TM_IMPL_H

#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <setjmp.h>
#include "_tm-word.h"

#ifndef TM_READSET_SIZE
#define TM_READSET_SIZE 256
#endif

#ifndef TM_WRITESET_SIZE
#define TM_WRITESET_SIZE 256
#endif

#ifndef TM_STACK_SIZE
#define TM_STACK_SIZE 256
#endif

#ifndef TM_STACK_ALIGNMENT
#define TM_STACK_ALIGNMENT 4
#endif

#ifdef TM_USE_CPP_ATOMIC
#include "_tm-impl-cpp.h"
#elif __SCC__
#include "_tm-impl-scc.h"
#else
#error unknown compiler
#endif 

struct tm_transaction
{
        unsigned readset_pos;
        unsigned writeset_pos;
        unsigned stack_pos;
};

struct tm_readset
{
        tm_versioned_lock* locks[TM_READSET_SIZE];
        unsigned size;
};

struct tm_writeset_entry
{
        struct tm_writeset_entry* next;
        struct tm_writeset_entry* prev;
        _tm_word* address;
        _tm_word value;
        unsigned mask;
};

struct tm_writeset_bucket
{
        struct tm_writeset_entry* head;
        struct tm_writeset_entry* tail;
};

struct tm_writeset
{
        // todo: split?
        struct tm_writeset_entry entries[TM_WRITESET_SIZE];
        struct tm_writeset_bucket lookup[TM_WRITESET_SIZE];
        unsigned size;
};

struct tm_memcutter
{
        const _tm_word* source_pos;
        _tm_word* dest_pos;
        unsigned mask;
        unsigned pos;
        unsigned num_words;
        unsigned last_word_mask;
};

struct tm_stack
{
        char chunk[TM_STACK_SIZE];
        unsigned size;
};

static inline _tm_word* tm_get_word_ptr(void* ptr)
{
        return (_tm_word*)((size_t)ptr & ~(sizeof(_tm_word) - 1));
}

static inline const _tm_word* tm_get_word_ptr_c(const void* ptr)
{
        return (const _tm_word*)((size_t)ptr & ~(sizeof(_tm_word) - 1));
}

static inline void tm_write_word(const _tm_word* source, _tm_word* dest, unsigned mask)
{
        if (mask == TM_MAX_WORD_MASK)
        {
                *dest = *source;
                return;
        }
        // todo: optimize?
        for (unsigned i = 0; i < sizeof(_tm_word); i++)
        {
                const char* source_byte = (const char*)source + i;
                char* dest_byte = (char*)dest + i;
                if (mask & (1 << i))
                        *dest_byte = *source_byte;
        }
}

static inline void tm_release_lock(tm_versioned_lock* lock)
{
        tm_set_lock(lock, tm_sample_lock(lock) & ~1);
}

static inline int tm_write_word_atomic(
        tm_versioned_lock* lock, const _tm_word* source, _tm_word* dest, unsigned mask)
{
        if (mask == TM_MAX_WORD_MASK)
        {
                *dest = *source;
                return 1;
        }

        if (!tm_acquire_lock(lock))
                return 0;
        tm_write_word(source, dest, mask);
        tm_release_lock(lock);
        return 1;
}

static inline int tm_readset_append(struct tm_readset* self, tm_versioned_lock* lock)
{
        if (self->size >= TM_READSET_SIZE)
                return 0;

        self->locks[self->size++] = lock;
        return 1;
}

static inline struct tm_writeset_bucket* tm_writeset_get_bucket(
        struct tm_writeset* self, const void* address)
{
        return self->lookup + ((size_t)address & (TM_WRITESET_SIZE - 1));
}

static inline int tm_writeset_append(
        struct tm_writeset* self, _tm_word* address, _tm_word value, unsigned mask)
{
        if (self->size >= TM_WRITESET_SIZE)
                return 0;

        struct tm_writeset_entry* e = self->entries + self->size++;
        e->address = address;
        e->value = value;
        e->mask = mask;
        e->next = 0;

        struct tm_writeset_bucket* b = tm_writeset_get_bucket(self, address);
        if (b->tail)
        {
                e->prev = b->tail;
                b->tail->next = e;
                b->tail = e;
        }
        else
        {
                e->prev = 0;
                b->head = e;
                b->tail = e;
        }

        return 1;
}

static inline unsigned tm_maybe_read_part_from_writeset(
        struct tm_writeset* self, const _tm_word* source, _tm_word* dest, unsigned mask)
{
        if (!self->size)
                return mask;

        struct tm_writeset_bucket* b = tm_writeset_get_bucket(self, source);
        if (!b->head)
                return mask;

        unsigned read_mask = 0;
        for (struct tm_writeset_entry* it = b->head; it; it = it->next)
        {
                unsigned intersection = it->mask & mask;
                if (it->address == source && intersection)
                {
                        tm_write_word(&it->value, dest, intersection);
                        read_mask |= intersection;
                }
        }
        return mask ^ read_mask;
}

static inline void tm_writeset_resize(struct tm_writeset* self, unsigned new_size)
{
        for (unsigned i = new_size; i < self->size; i++)
        {
                struct tm_writeset_entry* e = self->entries + i;
                struct tm_writeset_bucket* b = tm_writeset_get_bucket(self, e->address);

                if (e == b->head)
                        b->head = e->next;
                if (e == b->tail)
                        b->tail = e->prev;
                
                if (e->next)
                        e->next->prev = e->prev;
                if (e->prev)
                        e->prev->next = e->next;
        }
        self->size = new_size;
}

static void tm_fatal_error(int code)
{
        printf("libtm fatal error: %d\n", code);
        exit(code);
}

static void tm_memcutter_init(
        struct tm_memcutter* self, const void* source, void* dest, unsigned n, int init_for_write)
{
        size_t offset;
        self->last_word_mask = 0;
        if (init_for_write)
        {
                offset = (size_t)dest & (sizeof(_tm_word) - 1);
                self->dest_pos = tm_get_word_ptr(dest);
                self->source_pos = (const _tm_word*)((const char*)source - offset);
                // todo: fix this -=V=-
                self->num_words = ((unsigned)tm_get_word_ptr((char*)dest + n) 
                        - (unsigned)self->dest_pos) / sizeof(_tm_word);
        }
        else
        {
                offset = (size_t)source & (sizeof(_tm_word) - 1);
                self->source_pos = tm_get_word_ptr_c(source);
                self->dest_pos = (_tm_word*)((char*)dest - offset);
                // todo: fix this -=V=-
                self->num_words = ((unsigned)tm_get_word_ptr_c((const char*)source + n) 
                        - (unsigned)self->source_pos) / sizeof(_tm_word);
        }

        if (!self->num_words)
                self->num_words = 1;

        self->pos = 0;

        if (offset + n > sizeof(_tm_word))
        {
                self->num_words++;
                unsigned rem = self->num_words * sizeof(_tm_word) - (offset + n);
                self->last_word_mask = (TM_MAX_WORD_MASK >> rem) & TM_MAX_WORD_MASK;
                self->mask = (TM_MAX_WORD_MASK << offset) & TM_MAX_WORD_MASK;
        }
        else
                self->mask = (TM_MAX_WORD_MASK << offset)
                        & (TM_MAX_WORD_MASK >> (sizeof(_tm_word) - (offset + n)));
}

static int tm_memcutter_advance(struct tm_memcutter* self)
{
        if (self->pos + 1 >= self->num_words)
                return 0;

        self->source_pos++;
        self->pos++;
        self->dest_pos++;

        // todo: fix this -=V-=
        if (self->pos == self->num_words - 1)
                self->mask = self->last_word_mask;
        else
                self->mask = TM_MAX_WORD_MASK;

        return 1;
}

static inline void* tm_alloca(struct tm_stack* self, size_t n)
{
        size_t padding = (size_t)(self->chunk + self->size) & (TM_STACK_ALIGNMENT - 1);
        if (n + padding + self->size >= TM_STACK_SIZE)
                return 0;

        self->size += padding;
        void* ptr = self->chunk + self->size;
        self->size += n;
        return ptr;
}

#endif