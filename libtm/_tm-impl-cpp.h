#ifndef _TM_IMPL_COMPILER_NAME
#define _TM_IMPL_COMPILER_NAME

#include <atomic>

typedef std::atomic<unsigned> tm_versioned_lock;

static inline unsigned tm_sample_lock(const tm_versioned_lock* lock)
{
        return *lock;
}

static inline unsigned tm_inc_version(tm_versioned_lock* lock)
{
        *lock += 2;
        return *lock;
}

static inline void tm_set_lock(tm_versioned_lock* lock, unsigned val)
{
        *lock = val;
}

static inline int tm_acquire_lock(tm_versioned_lock* lock)
{
        unsigned val = tm_sample_lock(lock) & ~1;
        return lock->compare_exchange_weak(val, val | 1);
}

#endif