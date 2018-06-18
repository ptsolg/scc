#ifndef THREAD_H
#define THREAD_H

static void thread_init(struct thread* self, void(*entry)(void*), void* data);
static int thread_start(struct thread* self);
static void thread_wait(const struct thread* self);

#if _WIN32 || _WIN64
#include "thread-win.h"
#elif __APPLE__
#include "thread-osx.h"
#else
#error todo
#endif

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