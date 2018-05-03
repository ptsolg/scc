#ifndef SSAIZER_H
#define SSAIZER_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-builder.h"
#include "scc/core/dseq-common.h"

typedef struct _ssa_context ssa_context;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_expr tree_expr;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_instr ssa_instr;
typedef struct _ssa_module ssa_module;
typedef struct _tree_decl tree_decl;
typedef struct _tree_module tree_module;
typedef struct _ssa_value ssa_value;
typedef struct _dseq strmap_stack;
typedef struct _dseq dseq;
typedef struct _htab ptrset;

typedef struct _ssaizer
{
        ssa_context* context;
        ssa_block* block;
        ssa_value* function;
        ssa_instr* alloca_insertion_pos;
        ssa_module* module;
        ssa_builder builder;

        // stack of strmap's used for tracking the last definition of the variable
        strmap_stack defs;
        strmap labels;
        strmap globals;
        ptrset emitted_records;

        dseq continue_stack;
        dseq break_stack;
        dseq switch_stack;
} ssaizer;

extern void ssaizer_init(ssaizer* self, ssa_context* context);
extern void ssaizer_dispose(ssaizer* self);

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block);
extern void ssaizer_finish_block(ssaizer* self, ssa_block* block);
extern void ssaizer_finish_current_block(ssaizer* self);

extern ssa_block* ssaizer_new_block(ssaizer* self);
extern bool ssaizer_current_block_is_terminated(const ssaizer* self);

extern void ssaizer_push_scope(ssaizer* self);
extern void ssaizer_pop_scope(ssaizer* self);

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* def);
extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var);

extern void ssaizer_set_global_decl(ssaizer* self, tree_decl* decl, ssa_value* val);
extern ssa_value* ssaizer_get_global_decl(ssaizer* self, const tree_decl* decl);
extern bool ssaizer_record_is_emitted(const ssaizer* self, const tree_decl* record);
extern void ssaizer_set_record_emitted(ssaizer* self, const tree_decl* record);

extern ssa_block* ssaizer_get_label_block(ssaizer* self, const tree_decl* label);

extern void ssaizer_push_continue_dest(ssaizer* self, ssa_block* block);
extern void ssaizer_push_break_dest(ssaizer* self, ssa_block* block);
extern void ssaizer_push_switch_instr(ssaizer* self, ssa_instr* switch_instr);
extern void ssaizer_pop_continue_dest(ssaizer* self);
extern void ssaizer_pop_break_dest(ssaizer* self);
extern void ssaizer_pop_switch_instr(ssaizer* self);
extern ssa_block* ssaizer_get_continue_dest(ssaizer* self);
extern ssa_block* ssaizer_get_break_dest(ssaizer* self);
extern ssa_instr* ssaizer_get_switch_instr(ssaizer* self);

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module);

#ifdef __cplusplus
}
#endif

#endif