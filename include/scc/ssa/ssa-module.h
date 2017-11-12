#ifndef SSA_MODULE_H
#define SSA_MODULE_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_block ssa_block;
typedef struct _tree_decl tree_decl;

typedef struct _ssa_module
{
        htab _defs;
} ssa_module;

extern ssa_module* ssa_new_module(ssa_context* context);

extern serrcode ssa_module_add_def(ssa_module* self, tree_id id, ssa_block* def);
extern serrcode ssa_module_add_func_def(ssa_module* self, const tree_decl* func, ssa_block* def);
extern ssa_block* ssa_module_find_def(ssa_module* self, tree_id id);
extern ssa_block* ssa_module_find_func_def(ssa_module* self, const tree_decl* func);

#ifdef __cplusplus
}
#endif

#endif