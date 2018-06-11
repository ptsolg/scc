#ifndef _TM_IMPL_COMPILER_NAME
#define _TM_IMPL_COMPILER_NAME

typedef unsigned tm_versioned_lock;

static inline unsigned tm_sample_lock(const tm_versioned_lock* lock)
{
        unsigned v = *(const volatile tm_versioned_lock*)lock;
        __atomic_fence_st_seq_cst();
        return v;
}

static inline unsigned tm_inc_version(tm_versioned_lock* lock)
{
        return __atomic_add_fetch_32_seq_cst(lock, 2) + 2;
}

static inline void tm_set_lock(tm_versioned_lock* lock, unsigned val)
{
        __atomic_xchg_32_seq_cst(lock, val);
}

static inline int tm_acquire_lock(tm_versioned_lock* lock)
{
        unsigned val = tm_sample_lock(lock) & ~1;
        return __atomic_cmpxchg_32_weak_seq_cst(lock, val, val | 1);
}

#endif