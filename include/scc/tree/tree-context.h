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

typedef struct _tree_target_info tree_target_info;

typedef struct _tree_context
{
        bp_allocator _base;
        strpool _strings;
        tree_target_info* _target;
} tree_context;

extern void tree_init(tree_context* self, tree_target_info* target);
extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc);
extern void tree_dispose(tree_context* self);

static inline allocator* tree_get_allocator(const tree_context* self)
{
        return bpa_alloc(&self->_base);
}

static inline tree_target_info* tree_get_target(const tree_context* self)
{
        return self->_target;
}

static inline void* tree_allocate(tree_context* self, ssize bytes)
{
        return bp_allocate(&self->_base, bytes);
}

static inline const char* tree_get_id_cstr(const tree_context* self, tree_id id)
{
        return strpool_get(&self->_strings, id);
}

static inline tree_id tree_get_id(tree_context* self, const char* string, ssize len)
{
        return strpool_insertl(&self->_strings, string, len);
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_CONTEXT_H
