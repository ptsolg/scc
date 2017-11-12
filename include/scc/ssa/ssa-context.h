#ifndef SSA_CONTEXT_H
#define SSA_CONTEXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include <setjmp.h>

typedef struct _ssa_context
{
        bp_allocator _base;
        nnull_allocator _alloc;
} ssa_context;

extern void ssa_init_context(ssa_context* self, jmp_buf* on_out_of_mem);
extern void ssa_init_context_ex(ssa_context* self, jmp_buf* on_out_of_mem, allocator* alloc);
extern void ssa_dispose_context(ssa_context* self);

static inline void* ssa_allocate(ssa_context* self, ssize bytes)
{
        return bp_allocate(&self->_base, bytes);
}

static inline allocator* ssa_get_context_alloc(const ssa_context* self)
{
        return bpa_alloc(&self->_base);
}

#ifdef __cplusplus
}
#endif

#endif