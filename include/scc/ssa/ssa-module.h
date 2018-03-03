#ifndef SSA_MODULE_H
#define SSA_MODULE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "scc/core/dseq-common.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_function ssa_function;
typedef struct _ssa_value ssa_value;
typedef struct _tree_decl tree_decl;
typedef struct _dseq dseq;

typedef struct _ssa_module
{
        strmap lookup;
        list_head defs;
        dseq globals;
} ssa_module;

extern ssa_module* ssa_new_module(ssa_context* context);

extern void ssa_module_add_func_def(ssa_module* self, ssa_function* func);
extern void ssa_module_add_global_value(ssa_module* self, ssa_value* global);
extern ssa_function* ssa_module_lookup(const ssa_module* self, const tree_decl* func);
extern ssa_value** ssa_get_module_globals_begin(const ssa_module* self);
extern ssa_value** ssa_get_module_globals_end(const ssa_module* self);

static inline ssa_function* ssa_get_module_defs_begin(const ssa_module* self)
{
        return (ssa_function*)list_begin(&self->defs);
}

static inline ssa_function* ssa_get_module_defs_end(ssa_module* self)
{
        return (ssa_function*)list_end(&self->defs);
}

static inline const ssa_function* ssa_get_module_defs_cend(const ssa_module* self)
{
        return (const ssa_function*)list_cend(&self->defs);
}

#define SSA_FOREACH_MODULE_DEF(PMODULE, ITNAME) \
        for (ssa_function* ITNAME = ssa_get_module_defs_begin(PMODULE); \
                ITNAME != ssa_get_module_defs_cend(PMODULE); \
                ITNAME = ssa_get_next_function(ITNAME))

#define SSA_FOREACH_MODULE_GLOBAL(PMODULE, ITNAME, ENDNAME) \
        for (ssa_value** ITNAME = ssa_get_module_globals_begin(PMODULE), \
                **ENDNAME = ssa_get_module_globals_end(PMODULE); \
                ITNAME != ENDNAME; ITNAME++)

#ifdef __cplusplus
}
#endif

#endif