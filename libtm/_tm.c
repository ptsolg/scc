#include "_tm.h"

#define TM_MEMLOCKS_SIZE 256
#define TM_READSET_SIZE 256
#define TM_WRITESET_SIZE 256
#define TM_MAX_NESTING 256
#define TM_STACK_SIZE 256

enum
{
        TM_ERROR_NONE,
        TM_ERROR_READSET_FULL,
        TM_ERROR_WRITESET_FULL,
        TM_ERROR_STACK_OVERFLOW,
        TM_ERROR_TOO_DEEP_NESTING,
};

#include "_tm-impl.h"

static tm_versioned_lock global_version;
static tm_versioned_lock memlocks[TM_MEMLOCKS_SIZE];

static thread_local unsigned acquired_locks[TM_MEMLOCKS_SIZE];
static thread_local unsigned read_version;
static thread_local unsigned write_version;
static thread_local struct tm_readset readset;
static thread_local struct tm_writeset writeset;
static thread_local struct tm_stack stack;
static thread_local jmp_buf onabort;
static thread_local unsigned nesting;
static thread_local struct tm_transaction transactions[TM_MAX_NESTING];

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
                tm_fatal_error(TM_ERROR_READSET_FULL);
}

extern _tm_word _tm_read_word(const void* source)
{
        _tm_word result;
        if ((size_t)source & (sizeof(_tm_word) - 1))
                _tm_read(source, &result, sizeof(_tm_word));
        else
                tm_read(source, &result, TM_MAX_WORD_MASK);
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

extern void _tm_write_word(void* dest, _tm_word word)
{
        if ((size_t)dest & (sizeof(_tm_word) - 1))
                _tm_write(&word, dest, sizeof(_tm_word));
        else if (!tm_writeset_append(&writeset, dest, word, TM_MAX_WORD_MASK))
                tm_fatal_error(TM_ERROR_WRITESET_FULL);
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
                        tm_fatal_error(TM_ERROR_WRITESET_FULL);
        }
        while (tm_memcutter_advance(&mc));
}

extern int* _tm_start()
{
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
        t->readset_pos = readset.size;
        t->writeset_pos = writeset.size;
        t->stack_pos = stack.size;
        return jbuf;
}

extern void _tm_end()
{
        struct tm_transaction* t = tm_get_current_transaction();
        readset.size = t->readset_pos;
        tm_writeset_resize(&writeset, t->writeset_pos);
        stack.size = t->stack_pos;
        nesting--;
}

extern void _tm_abort()
{
        nesting = 0;
        readset.size = 0;
        tm_writeset_resize(&writeset, 0);
        stack.size = 0;
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
                stack.size = t->stack_pos;
                nesting--;
                return;
        }

        if (!writeset.size)
        {
                _tm_end();
                return;
        }

        unsigned entries_acquired = 0;

        // todo: skip addresses from previous transactions' stack?
        for (unsigned i = 0; i < writeset.size; i++)
        {
                if (!tm_acquire_lock_once((writeset.entries + i)->address))
                        tm_update_and_abort(entries_acquired);

                entries_acquired++;
        }

        write_version = tm_inc_version(&global_version);

        if (read_version + 1 != write_version)
                for (unsigned i = 0; i < readset.size; i++)
                {
                        unsigned version = tm_sample_lock(readset.locks[i]) & ~1;
                        if (version > read_version)
                                tm_update_and_abort(entries_acquired);
                }

        for (unsigned i = 0; i < writeset.size; i++)
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