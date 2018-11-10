#ifndef TREE_MODULE_H
#define TREE_MODULE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "decl.h"

typedef struct _tree_target_info tree_target_info;

typedef struct _tree_module
{
        tree_decl_scope globals;
        tree_target_info* target;
        tree_context* context;
} tree_module;

extern tree_module* tree_new_module(tree_context* context);

extern tree_decl* tree_module_lookup(const tree_module* self, tree_lookup_kind lk, tree_id id);
extern tree_decl* tree_module_lookup_s(const tree_module* self, tree_lookup_kind lk, const char* name);

static TREE_INLINE tree_decl_scope* tree_get_module_globals(tree_module* self)
{
        return &self->globals;
}

static TREE_INLINE const tree_decl_scope* tree_get_module_cglobals(const tree_module* self)
{
        return &self->globals;
}

static TREE_INLINE tree_target_info* tree_get_module_target(const tree_module* self)
{
        return self->target;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_MODULE_H
