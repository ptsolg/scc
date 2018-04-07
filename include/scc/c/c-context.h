#ifndef C_CONTEXT_H
#define C_CONTEXT_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/c/c-source.h"
#include "scc/c/c-lang-opts.h"
#include <setjmp.h>

typedef struct _tree_context tree_context;

typedef struct _c_context
{
        mempool memory;
        obstack nodes;
        tree_context* tree;
        c_source_manager source_manager;
        c_lang_opts lang_opts;
} c_context;

extern void c_context_init(
        c_context* self, 
        tree_context* tree, 
        file_lookup* lookup,
        jmp_buf on_bad_alloc);

extern void c_context_init_ex(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        jmp_buf on_bad_alloc,
        allocator* alloc);

extern void c_context_dispose(c_context* self);

static inline allocator* c_context_get_allocator(c_context* self)
{
        return mempool_to_allocator(&self->memory);
}

static inline void* c_context_allocate_node(c_context* self, size_t bytes)
{
        return obstack_allocate(&self->nodes, bytes);
}

static inline void* c_context_allocate(c_context* self, size_t bytes)
{
        return mempool_allocate(&self->memory, bytes);
}

static inline void c_context_deallocate(c_context* self, void* block)
{
        mempool_deallocate(&self->memory, block);
}

#ifdef __cplusplus
}
#endif

#endif
