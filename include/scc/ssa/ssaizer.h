#ifndef SSAIZER_H
#define SSAIZER_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-builder.h"
#include "ssa-function.h"

typedef struct _ssa_context ssa_context;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_expr tree_expr;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_module ssa_module;
typedef struct _tree_decl tree_decl;
typedef struct _tree_module tree_module;
typedef struct _ssa_value ssa_value;

typedef struct _ssaizer
{
        ssa_context* context;
        ssa_block* block;
        ssa_function* function;
        ssa_builder builder;

        // stack of htab's used for tracking the last definition of the variable
        dseq defs;
} ssaizer;

extern void ssaizer_init(ssaizer* self, ssa_context* context);
extern void ssaizer_dispose(ssaizer* self);

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block);
extern void ssaizer_finish_block(ssaizer* self, ssa_block* block);
extern void ssaizer_finish_current_block(ssaizer* self);

extern ssa_block* ssaizer_new_block(ssaizer* self);

extern void ssaizer_push_scope(ssaizer* self);
extern void ssaizer_pop_scope(ssaizer* self);

extern void ssaizer_set_lvalue_def(ssaizer* self, const tree_decl* var, ssa_value* def);
extern ssa_value* ssaizer_get_lvalue_def(ssaizer* self, const tree_decl* var);

extern void ssaizer_set_rvalue_def(ssaizer* self, const tree_decl* var, ssa_value* def);
extern ssa_value* ssaizer_get_rvalue_def(ssaizer* self, const tree_decl* var);

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module);

#ifdef __cplusplus
}
#endif

#endif