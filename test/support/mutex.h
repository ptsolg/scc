#ifndef MUTEX_H
#define MUTEX_H

struct mutex
{
        unsigned val;
};

static void mutex_init(struct mutex* self)
{
        self->val = 0;
}

static void mutex_lock(struct mutex* self)
{
        while (!__atomic_cmpxchg_32_weak_seq_cst(&self->val, 0, 1))
                ;
}

static void mutex_release(struct mutex* self)
{
        self->val = 0;
}

#endif