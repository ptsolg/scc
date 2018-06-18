#ifndef THREAD_OSX_H
#define THREAD_OSX_H

#include <pthread.h>

struct thread
{
        pthread_t thread;
        void* data;
        void(*entry)(void*);
};

static void thread_init(struct thread* self, void(*entry)(void*), void* data)
{
        self->thread = 0;
        self->entry = entry;
        self->data = data;
}

static void* _thread_entry(void* param)
{
        struct thread* self = param;
        self->entry(self->data);
        return 0;
}

static int thread_start(struct thread* self)
{
        return pthread_create(&self->thread, 0, &_thread_entry, self);
}

static void thread_wait(const struct thread* self)
{
        pthread_join(self->thread, 0);
}

#endif