#ifndef SSA_IR_H
#define SSA_IR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssa-common.h"
#include "ssa-builder.h"

typedef struct _ssa_context ssa_context;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_expr tree_expr;
typedef struct _ssa_block ssa_block;
typedef struct _ssa_module ssa_module;
typedef struct _tree_decl tree_decl;
typedef struct _tree_module tree_module;
typedef struct _ssa_value ssa_value;

// The structure used for generating intermediate representation(three-address code) from tree.
// The result will be used later for generating SSA.
typedef struct _ssa_ir_gen
{
        ssa_context* context;
        ssa_block* block;
        ssa_builder builder;

        // stack of htab's used for tracking the last definition of the variable
        dseq defs;

        ssa_id label_uid;
} ssa_ir_gen;

extern void ssa_init_ir_gen(ssa_ir_gen* self, ssa_context* context);
extern void ssa_dispose_ir_gen(ssa_ir_gen* self);

extern void ssa_ir_gen_enter_block(ssa_ir_gen* self, ssa_block* block);

extern void ssa_ir_gen_push_scope(ssa_ir_gen* self);
extern void ssa_ir_gen_pop_scope(ssa_ir_gen* self);

extern void ssa_ir_gen_set_def(ssa_ir_gen* self, const tree_decl* var, ssa_value* def);
extern ssa_value* ssa_ir_gen_get_def(ssa_ir_gen* self, const tree_decl* var);

extern ssa_value* ssa_gen_binary_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_unary_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_call_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_subscript_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_conditional_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_integer_literal_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_character_literal_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_floating_literal_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_string_literal_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_decl_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_member_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_cast_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_sizeof_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_paren_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_init_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_impl_init_expr_ir(ssa_ir_gen* self, const tree_expr* expr);
extern ssa_value* ssa_gen_expr_ir(ssa_ir_gen* self, const tree_expr* expr);

extern bool ssa_gen_var_decl_ir(ssa_ir_gen* self, const tree_decl* decl);
extern bool ssa_gen_decl_group_ir(ssa_ir_gen* self, const tree_decl* decl);
extern bool ssa_gen_decl_ir(ssa_ir_gen* self, const tree_decl* decl);

extern bool ssa_gen_labeled_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_default_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_case_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_compound_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_expr_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_if_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_switch_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_while_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_do_while_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_for_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_goto_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_continue_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_break_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_decl_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_return_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);
extern bool ssa_gen_stmt_ir(ssa_ir_gen* self, const tree_stmt* stmt);

extern ssa_module* ssa_gen_module_ir(ssa_ir_gen* self, const tree_module* module);

#ifdef __cplusplus
}
#endif

#endif