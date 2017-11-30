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
        htab _lookup;
        list_head _defs;
} ssa_module;

extern ssa_module* ssa_new_module(ssa_context* context);

extern void ssa_module_add_func_def(ssa_module* self, ssa_function* func);
extern ssa_function* ssa_module_lookup(ssa_module* self, const tree_decl* func);

static inline ssa_function* ssa_get_module_defs_begin(const ssa_module* self)
{
        return (ssa_function*)list_begin(&self->_defs);
}

static inline ssa_function* ssa_get_module_defs_end(ssa_module* self)
{
        return (ssa_function*)list_end(&self->_defs);
}

static inline const ssa_function* ssa_get_module_defs_cend(const ssa_module* self)
{
        return (const ssa_function*)list_cend(&self->_defs);
}

#define SSA_FOREACH_MODULE_DEF(PMODULE, ITNAME) \
        for (ssa_function* ITNAME = ssa_get_module_defs_begin(PMODULE); \
                ITNAME != ssa_get_module_defs_cend(PMODULE); \
                ITNAME = ssa_get_next_function(ITNAME))

#ifdef __cplusplus
}
#endif

#endif