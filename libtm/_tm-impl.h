#ifndef _TM_IMPL_H
#define _TM_IMPL_H

#ifndef TM_READSET_SIZE
#define TM_READSET_SIZE 8192
#endif

#ifndef TM_WRITESET_SIZE
#define TM_WRITESET_SIZE 8192
#endif

#ifndef TM_WRITEMAP_SIZE
#define TM_WRITEMAP_SIZE 8192
#endif

#ifdef TM_USE_CPP_ATOMIC
#include "_tm-impl-cpp.h"
#elif __SCC__
#if _M32
typedef unsigned size_t;
#else
typedef unsigned long long size_t;
#endif
#include "_tm-impl-scc.h"
#else
#error unknown compiler
#endif 

typedef unsigned tm_block;

struct tm_readset
{
        tm_versioned_lock* locks[TM_READSET_SIZE];
        unsigned size;
};

struct tm_writeset_entry
{
        struct tm_writeset_entry* prev;
        tm_versioned_lock* lock;
        union
        {
                tm_block* address;
                struct tm_writeset_entry** bucket;
        };
        int top_level;
        unsigned transaction_id;
        tm_block block;
        unsigned block_mask;
};

struct tm_writeset
{
        struct tm_writeset_entry entries[TM_WRITESET_SIZE];
        unsigned size;
};

struct tm_writemap
{
        struct tm_writeset_entry* buckets[TM_WRITEMAP_SIZE];
};

struct tm_memcutter
{
        const tm_block* source_pos;
        tm_block* dest_pos;
        unsigned mask;
        unsigned pos;
        unsigned num_blocks;
        unsigned off;
};

static inline tm_block* tm_get_block_ptr(void* ptr)
{
        return (tm_block*)((size_t)ptr & ~(sizeof(tm_block) - 1));
}

static inline const tm_block* tm_get_block_ptr_c(const void* ptr)
{
        return (const tm_block*)((size_t)ptr & ~(sizeof(tm_block) - 1));
}

static inline void tm_write_block(tm_block* dest, const tm_block* source, unsigned block_mask)
{
        // todo: optimize?
        for (unsigned i = 0; i < 4; i++)
        {
                const char* source_byte = (const char*)source + i;
                char* dest_byte = (char*)dest + i;
                if (block_mask & (1 << i))
                        *dest_byte = *source_byte;
        }
}

static inline void tm_release_lock(tm_versioned_lock* lock)
{
        tm_set_lock(lock, tm_sample_lock(lock) & ~1);
}

static inline int tm_readset_append(struct tm_readset* self, tm_versioned_lock* lock)
{
        if (self->size > TM_READSET_SIZE)
                return 0;

        self->locks[self->size++] = lock;
        return 1;
}

static inline struct tm_writeset_entry* tm_writeset_allocate_entry(struct tm_writeset* self)
{
        return self->size > TM_WRITESET_SIZE ? 0 : self->entries + self->size++;
}

static inline void tm_writeset_entry_add_block(
        struct tm_writeset_entry* self, const tm_block* source, unsigned block_mask)
{
        // todo: optimize?
        self->block_mask |= block_mask;

        for (unsigned i = 0; i < 4; i++)
        {
                const char* source_byte = (const char*)source + i;
                char* dest_byte = (char*)&self->block + i;
                if (block_mask & (1 << i))
                        *dest_byte = *source_byte;
        }
}

static inline struct tm_writeset_entry** tm_writemap_find_bucket(
        struct tm_writemap* self, const tm_block* block)
{
        size_t i = 1;
        size_t start = ((size_t)block) & (TM_WRITEMAP_SIZE - 1);
        size_t bucket_no = start;

        while (1)
        {
                struct tm_writeset_entry** bucket = self->buckets + bucket_no;
                if (!*bucket || (*bucket)->address == block)
                        return bucket;

                bucket_no += i++;
                bucket_no &= (TM_WRITEMAP_SIZE - 1);

                if (bucket_no == start)
                {
                        // if we are here, then writemap has 0 free slots
                        return 0;
                }
        }
}

static inline struct tm_writeset_entry* tm_writemap_find(
        struct tm_writemap* self, const tm_block* block)
{
        struct tm_writeset_entry** bucket = tm_writemap_find_bucket(self, block);
        return *bucket && (*bucket)->address == block ? *bucket : 0;
}

static void tm_fatal_error()
{
        // todo
}

static void tm_memcutter_init(
        struct tm_memcutter* self, const void* source, void* dest, unsigned n, int init_for_write)
{
        if (init_for_write)
        {
                self->off = (size_t)dest & (sizeof(tm_block) - 1);
                self->dest_pos = tm_get_block_ptr(dest);
                self->source_pos = (const tm_block*)((const char*)source - self->off);
                // todo: fix this -=V=-
                self->num_blocks = ((unsigned)tm_get_block_ptr((char*)dest + n) 
                        - (unsigned)self->dest_pos) / sizeof(tm_block);
        }
        else
        {
                self->off = (size_t)source & (sizeof(tm_block) - 1);
                self->source_pos = tm_get_block_ptr_c(source);
                self->dest_pos = (tm_block*)((char*)dest - self->off);
                // todo: fix this -=V=-
                self->num_blocks = ((unsigned)tm_get_block_ptr_c((const char*)source + n) 
                        - (unsigned)self->source_pos) / sizeof(tm_block);
        }

        self->pos = 0;

        if (self->off)
        {
                self->num_blocks++;
                self->mask = (15 << self->off) & 15;
        }
        else
                self->mask = 15;
}

static int tm_memcutter_advance(struct tm_memcutter* self)
{
        if (self->pos + 1 >= self->num_blocks)
                return 0;

        self->source_pos++;
        self->pos++;
        self->dest_pos++;

        if (self->off && self->pos == self->num_blocks - 1)
                self->mask = (15 >> (4 - self->off)) & 15;
        else
                self->mask = 15;

        return 1;
}

#endif