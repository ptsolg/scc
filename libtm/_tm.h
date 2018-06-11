#ifndef _TM_H
#define _TM_H

#if __SCC__
#define thread_local _Thread_local
#endif

#include "_tm-word.h"
#include <setjmp.h>

extern _tm_word _tm_read_word(const void* source);
extern void _tm_write_word(void* dest, _tm_word word);

extern void _tm_read(const void* source, void* dest, unsigned n);
extern void _tm_write(const void* source, void* dest, unsigned n);

// setjmp should be called right after _tm_start()
// returns 0 if transaction is nested
extern int* _tm_start();
extern void _tm_end();
extern void _tm_abort();
extern void _tm_commit();
extern void _tm_commit_n(size_t n);
extern void* _tm_alloca(size_t n);
extern int _tm_active();

#endif