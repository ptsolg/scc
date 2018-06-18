#include "_tm.h"
#include <string.h> // memcpy

#define TM_MEMLOCKS_SIZE (256U * 16)
#define TM_MAX_NESTING (256U)
#define TM_STACK_SIZE (256U)

enum
{
        TM_ERROR_NONE,
        TM_ERROR_READSET_FULL,
        TM_ERROR_WRITESET_FULL,
        TM_ERROR_STACK_OVERFLOW,
        TM_ERROR_TOO_DEEP_NESTING,
        TM_ERROR_OUT_OF_MEM,
};

#include "_tm-impl.h"

static tm_versioned_lock global_version;
static tm_versioned_lock* memlocks;
static struct tm_mutex init_lock;
static struct tm_mutex malloc_lock;

static thread_local unsigned* acquired_locks;
static thread_local unsigned read_version;
static thread_local unsigned write_version;
static thread_local struct tm_readset readset;
static thread_local struct tm_writeset writeset;
static thread_local struct tm_stack stack;
static thread_local jmp_buf onabort;
static thread_local unsigned nesting;
static thread_local struct tm_transaction* transactions;

static inline void* tm_malloc(size_t size)
{
        tm_mutex_lock(&malloc_lock);
        void* p = malloc(size);
        tm_mutex_unlock(&malloc_lock);
        return p;
}

static inline void tm_free(void* block)
{
        tm_mutex_lock(&malloc_lock);
        free(block);
        tm_mutex_unlock(&malloc_lock);
}

static inline size_t tm_get_lock_index(const void* p)
{
        return ((size_t)p & (TM_MEMLOCKS_SIZE - 1));
}

static inline tm_versioned_lock* tm_get_lock(const void* p)
{
        return memlocks + tm_get_lock_index(p);
}

static inline struct tm_transaction* tm_get_current_transaction()
{
        return transactions + nesting - 1;
}

static inline void tm_resize_readset()
{
        unsigned new_size = readset.size * 2;
        tm_versioned_lock** new_entries = (tm_versioned_lock**)tm_malloc(sizeof(tm_versioned_lock*) * new_size);
        if (!new_entries)
                tm_fatal_error(TM_ERROR_READSET_FULL);

        memcpy(new_entries, readset.locks, readset.size);
        
        tm_free(readset.locks);
        readset.locks = new_entries;
        readset.size = new_size;
}

static inline void tm_read(const _tm_word* source, _tm_word* dest, unsigned mask)
{
        mask = tm_maybe_read_part_from_writeset(&writeset, source, dest, mask);
        if (!mask)
                return;

        tm_versioned_lock* lock = tm_get_lock(source);
        unsigned sample = tm_sample_lock(lock);
        unsigned version = sample & ~1;

        if ((sample & 1) || version > read_version)
                _tm_abort();

        if (!tm_write_word_atomic(lock, source, dest, mask))
                _tm_abort();

        if (!tm_readset_append(&readset, lock))
        {
                tm_resize_readset();
                _tm_abort();
        }
}

extern _tm_word _tm_read_word(const void* source)
{
        _tm_word result;
        if ((size_t)source & (sizeof(_tm_word) - 1))
                _tm_read(source, &result, sizeof(_tm_word));
        else
                tm_read((const _tm_word*)source, &result, TM_MAX_WORD_MASK);
        return result;
}

extern void _tm_read(const void* source, void* dest, unsigned n)
{
        struct tm_memcutter mc;
        tm_memcutter_init(&mc, source, dest, n, 0);
        do
                tm_read(mc.source_pos, mc.dest_pos, mc.mask);
        while (tm_memcutter_advance(&mc));
}

static void tm_resize_writeset()
{
        unsigned new_size = writeset.size * 2;
        struct tm_writeset new_writeset;
        new_writeset.entries = (struct tm_writeset_entry*)tm_malloc(sizeof(struct tm_writeset_entry) * new_size);
        if (!new_writeset.entries)
                tm_fatal_error(TM_ERROR_WRITESET_FULL);

        new_writeset.lookup = (struct tm_writeset_bucket*)tm_malloc(sizeof(struct tm_writeset_bucket) * new_size);
        if (!new_writeset.lookup)
        {
                tm_free(new_writeset.entries);
                tm_fatal_error(TM_ERROR_WRITESET_FULL);
        }

        for (unsigned i = 0; i < new_size; i++)
        {
                new_writeset.lookup[i].head = 0;
                new_writeset.lookup[i].tail = 0;
        }

        new_writeset.size = new_size;
        new_writeset.pos = 0;

        for (unsigned i = 0; i < writeset.size; i++)
        {
                struct tm_writeset_entry* e = writeset.entries + i;
                tm_writeset_append(&new_writeset, e->address, e->value, e->mask);
        }

        tm_free(writeset.entries);
        tm_free(writeset.lookup);
        writeset = new_writeset;
}

extern void _tm_write_word(void* dest, _tm_word word)
{
        if ((size_t)dest & (sizeof(_tm_word) - 1))
                _tm_write(&word, dest, sizeof(_tm_word));
        else if (!tm_writeset_append(&writeset, (_tm_word*)dest, word, TM_MAX_WORD_MASK))
        {
                tm_resize_writeset();
                _tm_abort();
        }
}

extern void _tm_write(const void* source, void* dest, unsigned n)
{
        struct tm_memcutter mc;
        tm_memcutter_init(&mc, source, dest, n, 1);
        do
        {
                _tm_word value;
                if (!tm_write_word_atomic(tm_get_lock(mc.source_pos), mc.source_pos, &value, mc.mask))
                        _tm_abort();
                if (!tm_writeset_append(&writeset, mc.dest_pos, value, mc.mask))
                {
                        tm_resize_writeset();
                        _tm_abort();
                }
        }
        while (tm_memcutter_advance(&mc));
}

static void tm_init_shared_data()
{
        tm_mutex_lock(&init_lock);
        if (!memlocks)
        {
                memlocks = (tm_versioned_lock*)tm_malloc(sizeof(tm_versioned_lock) * TM_MEMLOCKS_SIZE);
                if (!memlocks)
                {
                        tm_mutex_unlock(&init_lock);
                        tm_fatal_error(TM_ERROR_OUT_OF_MEM);
                }
                for (unsigned i = 0; i < TM_MEMLOCKS_SIZE; i++)
                        memlocks[i] = 0;
        }
        tm_mutex_unlock(&init_lock);
}

static void tm_init_thread_local_data()
{
        acquired_locks = (unsigned*)tm_malloc(sizeof(unsigned) * TM_MEMLOCKS_SIZE);

        readset.size = 256;
        readset.locks = (tm_versioned_lock**)tm_malloc(sizeof(struct tm_versioned_lock*) * readset.size);

        writeset.size = 256;
        writeset.entries = (struct tm_writeset_entry*)tm_malloc(sizeof(struct tm_writeset_entry) * writeset.size);
        writeset.lookup = (struct tm_writeset_bucket*)tm_malloc(sizeof(struct tm_writeset_bucket) * writeset.size);
        for (unsigned i = 0; i < writeset.size; i++)
        {
                writeset.lookup[i].head = 0;
                writeset.lookup[i].tail = 0;
        }

        stack.chunk = (char*)tm_malloc(TM_STACK_SIZE);

        transactions = (struct tm_transaction*)tm_malloc(sizeof(struct tm_transaction) * TM_MAX_NESTING);

        if (!acquired_locks || !readset.locks 
                || !writeset.entries || !writeset.lookup 
                || !stack.chunk || !transactions)
        {
                tm_fatal_error(TM_ERROR_OUT_OF_MEM);
        }

        for (unsigned i = 0; i < TM_MEMLOCKS_SIZE; i++)
                acquired_locks[i] = 0;
}

extern int* _tm_start()
{
        if (!memlocks)
                tm_init_shared_data();
        if (!acquired_locks)
                tm_init_thread_local_data();

        if (nesting >= TM_MAX_NESTING)
                tm_fatal_error(TM_ERROR_TOO_DEEP_NESTING);

        int* jbuf = 0;
        if (!nesting)
        {
                jbuf = onabort;
                read_version = tm_sample_lock(&global_version);
        }

        nesting++;
        struct tm_transaction* t = tm_get_current_transaction();
        t->readset_pos = readset.pos;
        t->writeset_pos = writeset.pos;
        t->stack_pos = stack.pos;
        return jbuf;
}

extern void _tm_end()
{
        struct tm_transaction* t = tm_get_current_transaction();
        readset.pos = t->readset_pos;
        tm_writeset_resize(&writeset, t->writeset_pos);
        stack.pos = t->stack_pos;
        nesting--;
}

extern void _tm_abort()
{
        nesting = 0;
        readset.pos = 0;
        tm_writeset_resize(&writeset, 0);
        stack.pos = 0;
        longjmp(onabort, 0);
}

static int tm_acquire_lock_once(const void* address)
{
        size_t index = tm_get_lock_index(address);
        if (acquired_locks[index])
        {
                acquired_locks[index]++;
                return 1;
        }

        //todo: spin?
        if (!tm_acquire_lock(memlocks + index))
                return 0;

        acquired_locks[index] = 1;
        return 1;
}

static void tm_update_lock_once(const void* address, unsigned val)
{
        size_t index = tm_get_lock_index(address);
        unsigned n = acquired_locks[index];
        if (!n)
                return;

        if (n == 1)
                tm_set_lock(memlocks + index, val);

        acquired_locks[index]--;
}

static void tm_update_and_abort(unsigned entries_acquired)
{
        for (unsigned i = 0; i < entries_acquired; i++)
                tm_update_lock_once((writeset.entries + i)->address, write_version);
        _tm_abort();
}

extern void _tm_commit_n(size_t n)
{
        for (size_t i = 0; i < n; i++)
                _tm_commit();
}

extern void _tm_commit()
{
        if (nesting > 1)
        {
                struct tm_transaction* t = tm_get_current_transaction();
                stack.pos = t->stack_pos;
                nesting--;
                return;
        }

        if (!writeset.pos)
        {
                _tm_end();
                return;
        }

        unsigned entries_acquired = 0;

        // todo: skip addresses from previous transactions' stack?
        for (unsigned i = 0; i < writeset.pos; i++)
        {
                if (!tm_acquire_lock_once((writeset.entries + i)->address))
                        tm_update_and_abort(entries_acquired);

                entries_acquired++;
        }

        write_version = tm_inc_version(&global_version);

        if (read_version + 1 != write_version)
                for (unsigned i = 0; i < readset.pos; i++)
                {
                        unsigned version = tm_sample_lock(readset.locks[i]) & ~1;
                        if (version > read_version)
                                tm_update_and_abort(entries_acquired);
                }

        for (unsigned i = 0; i < writeset.pos; i++)
        {
                struct tm_writeset_entry* e = writeset.entries + i;
                tm_write_word(&e->value, e->address, e->mask);
                tm_update_lock_once(e->address, write_version);
        }

        _tm_end();
}

extern void* _tm_alloca(size_t n)
{
        return tm_alloca(&stack, n);
}

extern int _tm_active()
{
        return nesting != 0;
}