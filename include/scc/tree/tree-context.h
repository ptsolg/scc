#ifndef TREE_CONTEXT_H
#define TREE_CONTEXT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "scc/scl/alloc.h"
#include "tree-common.h"

typedef struct _tree_context
{
        bp_allocator _base;
        strpool _strings;
} tree_context;

extern void tree_init_context(tree_context* self);
extern void tree_init_context_ex(tree_context* self, allocator* alloc);
extern void tree_dispose_context(tree_context* self);

static inline allocator* tree_get_context_allocator(const tree_context* self)
{
        return bpa_alloc(&self->_base);
}

static inline strpool* tree_get_context_strings(tree_context* self)
{
        return &self->_strings;
}

static inline void* tree_allocate(tree_context* self, ssize bytes)
{
        return bp_allocate(&self->_base, bytes);
}

static inline const char* tree_get_id_cstr(const tree_context* self, tree_id id)
{
        return strpool_get(&self->_strings, id);
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_CONTEXT_H
