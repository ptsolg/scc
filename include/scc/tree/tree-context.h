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
        bump_ptr_allocator _alloc;
        strpool _strings;
        tree_target_info* _target;
} tree_context;

extern void tree_init(tree_context* self, tree_target_info* target);
extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc);
extern void tree_dispose(tree_context* self);

static inline allocator* tree_get_allocator(const tree_context* self)
{
        return bump_ptr_allocator_parent(&self->_alloc);
}

static inline tree_target_info* tree_get_target(const tree_context* self)
{
        return self->_target;
}

static inline void* tree_allocate(tree_context* self, ssize bytes)
{
        return bump_ptr_allocate(&self->_alloc, bytes);
}

static inline bool tree_get_id_strentry(
        const tree_context* self, tree_id id, strentry* result)
{
        return strpool_get(&self->_strings, id, result);
}

static inline const char* tree_get_id_cstr(const tree_context* self, tree_id id)
{
        strentry entry;
        return tree_get_id_strentry(self, id, &entry)
                ? (const char*)entry.data
                : NULL;
}

static inline tree_id tree_get_id(tree_context* self, const char* string, ssize len)
{
        return strpool_insert(&self->_strings, string, len);
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_CONTEXT_H
