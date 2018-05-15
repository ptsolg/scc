#include "_tm.h"
#include "_tm-impl.h"

#define TM_MEMLOCKS_SIZE 8192

static tm_versioned_lock global_version;
static tm_versioned_lock memlocks[TM_MEMLOCKS_SIZE];

thread_local unsigned _tm_transaction_id;

static thread_local unsigned read_version;
static thread_local unsigned write_version;
static thread_local struct tm_writemap writemap;
static thread_local struct tm_readset readset;
static thread_local struct tm_writeset writeset;

static inline tm_versioned_lock* tm_get_lock(const void* block)
{
        return memlocks + ((size_t)block & (TM_MEMLOCKS_SIZE - 1));
}

static inline int tm_read(const tm_block* source, tm_block* dest, unsigned block_mask)
{
        struct tm_writeset_entry* entry = tm_writemap_find(&writemap, source);
        if (entry && (entry->block_mask & block_mask) == block_mask)
        {
                tm_write_block(dest, &entry->block, block_mask);
                return 1;
        }

        tm_versioned_lock* lock = tm_get_lock(source);
        unsigned sample = tm_sample_lock(lock);
        unsigned version = sample & ~1;

        if ((sample & 1) || version > read_version)
                return 0;

        if (!tm_readset_append(&readset, lock))
                tm_fatal_error();

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
                tm_fatal_error();

        struct tm_writeset_entry* prev = *bucket;
        if (prev && prev->transaction_id == _tm_transaction_id)
        {
                tm_writeset_entry_add_block(prev, block, block_mask);
                return;
        }

        struct tm_writeset_entry* entry = tm_writeset_allocate_entry(&writeset);
        if (!entry)
                tm_fatal_error();

        entry->prev = prev;
        entry->lock = tm_get_lock(dest);
        entry->address = dest;
        entry->transaction_id = _tm_transaction_id;
        entry->block = *block;
        entry->block_mask = block_mask;

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
        self->locks_acquired = 0;
        self->writeset_pos = writeset.size;
        self->readset_pos = readset.size;
        read_version = global_version;
        self->read_version = read_version;
}

static void tm_transaction_reset(struct _tm_transaction* self, int reset_all)
{
        for (unsigned i = self->writeset_pos; i < writeset.size; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i;
                entry->bucket = tm_writemap_find_bucket(&writemap, entry->address);
        }
        for (unsigned i = self->writeset_pos; i < writeset.size; i++)
        {
                struct tm_writeset_entry** bucket = writeset.entries[i].bucket;
                if (!*bucket)
                        continue;
                if (reset_all || (*bucket)->transaction_id == _tm_transaction_id)
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

static void tm_transaction_cancel(struct _tm_transaction* self, int cancel_all)
{
        for (unsigned i = 0; i < self->locks_acquired; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i + self->writeset_pos;
                if (!entry->top_level)
                        continue;
                tm_release_lock(entry->lock);
        }
        tm_transaction_reset(self, cancel_all);
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
                if (entry->top_level && !tm_acquire_lock(entry->lock))
                {
                        tm_transaction_cancel(self, 1);
                        return 0;
                }
                self->locks_acquired++;
        }

        write_version = tm_inc_version(&global_version);

        for (unsigned i = 0; i < readset.size; i++)
        {
                unsigned version = tm_sample_lock(readset.locks[i]) & ~1;
                if (version > self->read_version)
                {
                        tm_transaction_cancel(self, 1);
                        return 0;
                }
        }

        for (unsigned i = 0; i < writeset.size; i++)
        {
                struct tm_writeset_entry* entry = writeset.entries + i;
                if (!entry->top_level)
                        continue;
                tm_write_block(entry->address, &entry->block, entry->block_mask);
                tm_set_lock(entry->lock, write_version);
        }

        tm_transaction_reset(self, 1);
        return 1;
}

extern void _tm_transaction_reset(struct _tm_transaction* self)
{
        tm_transaction_reset(self, 0);
}

extern void _tm_transaction_cancel(struct _tm_transaction* self)
{
        tm_transaction_cancel(self, 0);
}
