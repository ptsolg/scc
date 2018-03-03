#ifndef C_CONTEXT_H
#define C_CONTEXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/tree/tree.h"
#include <stdio.h>
#include <setjmp.h>

typedef struct _c_source c_source;
typedef struct _file_entry file_entry;

typedef struct _c_context
{
        mempool memory;
        obstack nodes;
        list_head sources;
        tree_context* tree;
} c_context;

extern void c_context_init(c_context* self, tree_context* tree, jmp_buf on_bad_alloc);
extern void c_context_init_ex(c_context* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc);

extern void c_context_dispose(c_context* self);

extern c_source* c_new_source(c_context* context, file_entry* entry);
extern void c_delete_source(c_context* context, c_source* source);

static inline tree_context* c_context_get_tree_context(const c_context* self)
{
        return self->tree;
}

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
