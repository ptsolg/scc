#ifndef TREE_MODULE_H
#define TREE_MODULE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-decl.h"

typedef struct _tree_module
{
        tree_decl_scope     _globals;
        tree_platform_info* _platform;
} tree_module;

extern tree_module* tree_new_module(tree_context* context, tree_platform_info* platform);

static inline tree_decl_scope* tree_get_module_globals(tree_module* self)
{
        return &self->_globals;
}

static inline const tree_decl_scope* tree_get_module_cglobals(const tree_module* self)
{
        return &self->_globals;
}

static inline tree_platform_info* tree_get_module_platform(const tree_module* self)
{
        return self->_platform;
}

#ifdef __cplusplus
}
#endif

#endif // !TREE_MODULE_H