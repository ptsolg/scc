#ifndef C_SEMA_H
#define C_SEMA_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-error.h"
#include "c-context.h"

typedef struct _tree_stmt tree_stmt;
typedef struct _tree_decl tree_decl;
typedef struct _tree_target_info tree_target_info;
typedef struct _tree_module tree_module;
typedef struct _tree_scope tree_scope;
typedef struct _tree_decl_scope tree_decl_scope;
typedef struct _tree_type tree_type;
typedef struct _tree_expr tree_expr;

#define C_CASEMAP_EMPTY_KEY (-1)
#define C_CASEMAP_DELETED_KEY (-2)

#define HTAB_FN(N) c_casemap_##N
#define HTAB_TP    c_casemap
#define HTAB_ETP   c_casemap_entry
#define HTAB_KTP   uint32_t
#define HTAB_EK    C_CASEMAP_EMPTY_KEY
#define HTAB_DK    C_CASEMAP_DELETED_KEY
#define HTAB_VTP   void*
#include "scc/core/htab-type.h"

typedef struct
{
        tree_stmt* switch_stmt;
        c_casemap labels;
        bool has_default_label;
        bool in_atomic_block;
} c_switch_stmt;

#define VEC_FN(N) c_switch_stack_##N
#define VEC_TP    c_switch_stack
#define VEC_VTP   c_switch_stmt
#include "scc/core/vec-type.h"

// The structure, used for semantic analysis and building AST
typedef struct _c_sema
{
        tree_context* context;
        c_context* ccontext;
        c_logger* logger;
        tree_decl* function;
        tree_decl_scope* globals;
        tree_decl_scope* labels;
        tree_decl_scope* locals;
        c_switch_stack switch_stack;
        tree_scope* scope;
        tree_module* module;
        tree_target_info* target;

        struct
        {
                ptrvec non_atomic_gotos;
                strmap atomic_labels;
                int atomic_stmt_nesting;
        } tm_info;
} c_sema;

extern void c_sema_init(c_sema* self, c_context* context, c_logger* logger);
extern void c_sema_dispose(c_sema* self);

extern tree_module* c_sema_new_module(c_sema* self);
extern void c_sema_enter_scope(c_sema* self, tree_scope* scope);
extern void c_sema_exit_scope(c_sema* self);
extern void c_sema_enter_decl_scope(c_sema* self, tree_decl_scope* scope);
extern void c_sema_exit_decl_scope(c_sema* self);
extern void c_sema_enter_function(c_sema* self, tree_decl* func);
extern void c_sema_exit_function(c_sema* self);

// creates new decl scope, and enters it
// this is used when we need to create implicit decl scope.
// e.g: to hide for-loop variables from using them outside loop scope
extern void c_sema_push_scope(c_sema* self);

extern void c_sema_push_switch_stmt_info(c_sema* self, tree_stmt* switch_stmt);
extern void c_sema_pop_switch_stmt_info(c_sema* self);
extern void c_sema_set_switch_stmt_has_default_label(c_sema* self);
extern void c_sema_set_switch_stmt_in_atomic_block(c_sema* self);
extern c_switch_stmt* c_sema_get_switch_stmt_info(const c_sema* self);
extern bool c_sema_in_switch_stmt(const c_sema* self);
extern bool c_sema_switch_stmt_has_default_label(const c_sema* self);
extern bool c_sema_switch_stmt_in_atomic_block(const c_sema* self);
extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label);

extern bool c_sema_at_file_scope(const c_sema* self);
extern bool c_sema_at_block_scope(const c_sema* self);

extern bool c_sema_in_atomic_block(const c_sema* self);
extern bool c_sema_in_transaction_safe_function(const c_sema* self);
extern bool c_sema_in_transaction_safe_block(const c_sema* self);

#ifdef __cplusplus
}
#endif

#endif
