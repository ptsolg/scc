#include "_tm.h"

#define TM_MEMLOCKS_SIZE 8192
#define TM_READSET_SIZE 8192
#define TM_WRITESET_SIZE 8192
#define TM_WRITEMAP_SIZE 8192

#define ERR_NONE 0
#define ERR_READSET_FULL 1
#define ERR_WRITESET_FULL 2
#define ERR_WRITEMAP_FULL 3

#include "_tm-impl.h"

static tm_versioned_lock global_version;
static tm_versioned_lock memlocks[TM_MEMLOCKS_SIZE];

thread_local unsigned _tm_transaction_id;

static thread_local char acquired_locks[TM_MEMLOCKS_SIZE];
static thread_local unsigned read_version;
static thread_local unsigned write_version;
static thread_local struct tm_writemap writemap;
static thread_local struct tm_readset readset;
static thread_local struct tm_writeset writeset;

static inline size_t tm_get_lock_index(const void* block)
{
        return ((size_t)block & (TM_MEMLOCKS_SIZE - 1));
}

static inline tm_versioned_lock* tm_get_lock(const void* block)
{
        return memlocks + tm_get_lock_index(block);
}

static inline int tm_read(const tm_block* source, tm_block* dest, unsigned block_mask)
{
        struct tm_writeset_entry* entry = tm_writemap_find(&writemap, source);
        if (entry)
        {
                unsigned intersection = entry->block_mask & block_mask;
                if (intersection)
                {
                        tm_write_block(dest, &entry->block, intersection);
                        if (intersection == block_mask)
                                return 1;

                        block_mask ^= intersection;
                }
        }

        tm_versioned_lock* lock = tm_get_lock(source);
        unsigned sample = tm_sample_lock(lock);
        unsigned version = sample & ~1;

        if ((sample & 1) || version > read_version)
                return 0;

        if (!tm_readset_append(&readset, lock))
                tm_fatal_error(ERR_READSET_FULL);

        tm_write_block(dest, source, block_mask);
        return 1;
}

extern int _tm_read(const void* source, void* dest, unsigned n)
{
        if (source == dest)
                return 1;

        struct tm_memcutter mc;
        tm_memcutter_init(&mc, source, dest, n, 0);
        do
        {
                if (!tm_read(mc.source_pos, mc.dest_pos, mc.mask))
                        return 0;
        } while (tm_memcutter_advance(&mc));

        return 1;
}

static inline void tm_write(tm_block* dest, const tm_block* block, unsigned block_mask)
{
        struct tm_writeset_entry** bucket = tm_writemap_find_bucket(&writemap, dest);
        if (!bucket)
                tm_fatal_error(ERR_WRITEMAP_FULL);

        struct tm_writeset_entry* prev = *bucket;
        if (prev && prev->transaction_id == _tm_transaction_id)
        {
                tm_writeset_entry_add_block(prev, block, block_mask);
                return;
        }

        struct tm_writeset_entry* entry = tm_writeset_allocate_entry(&writeset);
        if (!entry)
                tm_fatal_error(ERR_WRITESET_FULL);

        entry->prev = prev;
        entry->lock = tm_get_lock(dest);
        entry->address = dest;
        entry->transaction_id = _tm_transaction_id;

        if (prev)
        {
                entry->block = prev->block;
                entry->block_mask = prev->block_mask | block_mask;
        }
        else
                entry->block_mask = block_mask;
        tm_write_block(&entry->block, block, block_mask);

        entry->top_level = 1;
        if (prev)
                prev->top_level = 0;

        *bucket = entry;
}

extern void _tm_write(const void* source, void* dest, unsigned n)
{
        if (source == dest)
                return;

        struct tm_memcutter mc;
        tm_memcutter_init(&mc, source, dest, n, 1);
        do
        {
                tm_write(mc.dest_pos, mc.source_pos, mc.mask);
        } while (tm_memcutter_advance(&mc));
}

extern void _tm_transaction_init(struct _tm_transaction* self)
{
        _tm_transaction_id++;
        self->writeset_entries_locked = 0;
        self->writeset_pos = writeset.size;
        self->readset_pos = readset.size;
        read_version = tm_sample_lock(&global_version);
        self->read_version = read_version;
}

static void tm_release_writeset_entry(struct tm_writeset_entry* entry)
{
        size_t index = tm_get_lock_index(entry->address);
        if (!acquired_locks[index])
                return;

        acquired_locks[index] = 0;
        tm_release_lock(entry->lock);
}

static void tm_transaction_cancel(struct _tm_transaction* self)
{
        for (unsigned i = 0; i < self->writeset_entries_locked; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i + self->writeset_pos;
                if (!entry->top_level)
                        continue;

                tm_release_writeset_entry(entry);
        }
        _tm_transaction_reset(self);
}

static int tm_acquire_writeset_entry(struct tm_writeset_entry* entry)
{
        size_t index = tm_get_lock_index(entry->address);
        if (acquired_locks[index])
                return 1;

        if (!tm_acquire_lock(entry->lock))
                return 0;

        acquired_locks[index] = 1;
        return 1;
}

extern int _tm_transaction_commit(struct _tm_transaction* self)
{
        if (_tm_transaction_id != 1)
        {
                _tm_transaction_id--;
                return 1;
        }

        for (unsigned i = 0; i < writeset.size; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i;
                if (entry->top_level && !tm_acquire_writeset_entry(entry))
                {
                        tm_transaction_cancel(self);
                        return 0;
                }
                self->writeset_entries_locked++;
        }

        write_version = tm_inc_version(&global_version);

        for (unsigned i = 0; i < readset.size; i++)
        {
                unsigned version = tm_sample_lock(readset.locks[i]) & ~1;
                if (version > self->read_version)
                {
                        tm_transaction_cancel(self);
                        return 0;
                }
        }

        for (unsigned i = 0; i < writeset.size; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i;
                if (!entry->top_level)
                        continue;

                tm_write_block(entry->address, &entry->block, entry->block_mask);
                acquired_locks[tm_get_lock_index(entry->address)] = 0;
                tm_set_lock(entry->lock, write_version);
        }

        _tm_transaction_reset(self);
        return 1;
}

extern void _tm_transaction_reset(struct _tm_transaction* self)
{
        for (unsigned i = self->writeset_pos; i < writeset.size; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i;
                entry->bucket = tm_writemap_find_bucket(&writemap, entry->address);
        }
        for (unsigned i = self->writeset_pos; i < writeset.size; i++)
        {
                struct tm_writeset_entry** bucket = writeset.entries[i].bucket;
                while (*bucket && (*bucket)->transaction_id >= _tm_transaction_id)
                {
                        *bucket = (*bucket)->prev;
                        if (*bucket)
                                (*bucket)->top_level = 1;
                }
        }

        writeset.size = self->writeset_pos;
        readset.size = self->readset_pos;
        _tm_transaction_id--;
        read_version = self->read_version;
}
