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
typedef struct _ssa_function ssa_function;
typedef struct _tree_decl tree_decl;

typedef struct _ssa_module
{
        htab _defs;
} ssa_module;

extern ssa_module* ssa_new_module(ssa_context* context);

extern void ssa_module_add_func_def(ssa_module* self, ssa_function* func);
extern ssa_function* ssa_module_find_func_def(ssa_module* self, const tree_decl* func);

static inline hiter ssa_get_module_begin(const ssa_module* self)
{
        return htab_begin(&self->_defs);
}

#ifdef __cplusplus
}
#endif

#endif