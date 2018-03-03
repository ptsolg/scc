#ifndef C_SEMA_H
#define C_SEMA_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-error.h"
#include "c-context.h"
#include "scc/core/dseq-common.h"

typedef struct _htab c_case_label_map;

typedef struct
{
        tree_stmt* switch_stmt;
        c_case_label_map labels;
        bool has_default_label;
} c_switch_stmt_info;

typedef struct _dseq c_switch_stack;

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
} c_sema;

extern void c_sema_init(c_sema* self, c_context* context, c_logger* logger);
extern void c_sema_dispose(c_sema* self);

extern void c_sema_enter_module(c_sema* self, tree_module* module);
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
extern c_switch_stmt_info* c_sema_get_switch_stmt_info(const c_sema* self);
extern bool c_sema_in_switch_stmt(const c_sema* self);
extern bool c_sema_switch_stmt_has_default_label(const c_sema* self);
extern bool c_sema_switch_stmt_register_case_label(const c_sema* self, tree_stmt* label);

extern bool c_sema_at_file_scope(const c_sema* self);
extern bool c_sema_at_block_scope(const c_sema* self);

#ifdef __cplusplus
}
#endif

#endif
