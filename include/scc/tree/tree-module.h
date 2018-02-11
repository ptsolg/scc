#ifndef TREE_MODULE_H
#define TREE_MODULE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-decl.h"

typedef struct _tree_target_info tree_target_info;

typedef struct _tree_module
{
        tree_decl_scope globals;
        tree_target_info* target;
} tree_module;

extern tree_module* tree_new_module(tree_context* context);

static inline tree_decl_scope* tree_get_module_globals(tree_module* self)
{
        return &self->globals;
}

static inline const tree_decl_scope* tree_get_module_cglobals(const tree_module* self)
{
        return &self->globals;
}

static inline tree_target_info* tree_get_module_target(const tree_module* self)
{
        return self->target;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_MODULE_H
