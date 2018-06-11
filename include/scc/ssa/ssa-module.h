#ifndef SSA_MODULE_H
#define SSA_MODULE_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "scc/core/vec.h"

typedef struct _ssa_context ssa_context;
typedef struct _ssa_value ssa_value;
typedef struct _tree_decl tree_decl;

typedef struct _ssa_module
{
        ptrvec globals;
        ptrvec type_decls;
} ssa_module;

extern ssa_module* ssa_new_module(ssa_context* context);

extern void ssa_add_module_global(ssa_module* self, ssa_value* val);
extern void ssa_add_module_type_decl(ssa_module* self, tree_decl* decl);
extern void ssa_number_module_values(ssa_module* self);

extern ssa_value** ssa_get_module_globals_begin(const ssa_module* self);
extern ssa_value** ssa_get_module_globals_end(const ssa_module* self);
extern tree_decl** ssa_get_module_type_decls_begin(const ssa_module* self);
extern tree_decl** ssa_get_module_type_decls_end(const ssa_module* self);

#define SSA_FOREACH_MODULE_GLOBAL(PMODULE, ITNAME, ENDNAME) \
        for (ssa_value** ITNAME = ssa_get_module_globals_begin(PMODULE), \
                **ENDNAME = ssa_get_module_globals_end(PMODULE); \
                ITNAME != ENDNAME; ITNAME++)

#define SSA_FOREACH_MODULE_TYPE_DECL(PMODULE, ITNAME, ENDNAME) \
        for (tree_decl** ITNAME = ssa_get_module_type_decls_begin(PMODULE), \
                **ENDNAME = ssa_get_module_type_decls_end(PMODULE); \
                ITNAME != ENDNAME; ITNAME++)

#ifdef __cplusplus
}
#endif

#endif