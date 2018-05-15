#ifndef THREAD_H
#define THREAD_H

#include <Windows.h>

struct thread
{
        void* handle;
        void* data;
        void(*entry)(void*);
};

static void thread_init(struct thread* self, void(*entry)(void*), void* data)
{
        self->handle = 0;
        self->entry = entry;
        self->data = data;
}

static unsigned long _thread_entry(void* param) _Stdcall
{
        struct thread* self = param;
        self->entry(self->data);
        ExitThread(0);
}

static int thread_start(struct thread* self)
{
        self->handle = CreateThread(0, 0, &_thread_entry, self, 0, 0);
        return self->handle != 0;
}

static void thread_wait(const struct thread* self)
{
        WaitForSingleObject(self->handle, INFINITE);
}

#define CREATE_THREADS(N, ENTRY, DATA) \
        do \
        { \
                struct thread ths[N]; \
                for (int i = 0; i < N; i++) \
                { \
                        thread_init(ths + i, ENTRY, DATA); \
                        thread_start(ths + i); \
                } \
                for (int i = 0; i < N; i++) \
                        thread_wait(ths + i); \
        } while (0)

#endif