#ifndef CSEMA_H
#define CSEMA_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-error.h"
#include "c-context.h"

typedef struct
{
        tree_stmt* switch_stmt;
        htab used_values;
        bool has_default_label;
} cswitch_stmt_info;

// The structure, used for semantic analysis and building AST
typedef struct _csema
{
        tree_context* context;
        ccontext* ccontext;
        clogger* logger;
        tree_decl* function;
        tree_decl_scope* globals;
        tree_decl_scope* labels;
        tree_decl_scope* locals;
        dseq switch_stack;
        tree_scope* scope;
        tree_module* module;
        tree_target_info* target;
} csema;

extern void csema_init(csema* self, ccontext* context, clogger* logger);
extern void csema_dispose(csema* self);

extern void csema_enter_module(csema* self, tree_module* module);
extern void csema_enter_scope(csema* self, tree_scope* scope);
extern void csema_exit_scope(csema* self);
extern void csema_enter_decl_scope(csema* self, tree_decl_scope* scope);
extern void csema_exit_decl_scope(csema* self);
extern void csema_enter_function(csema* self, tree_decl* func);
extern void csema_exit_function(csema* self);

// creates new decl scope, and enters it
// this is used when we need to create implicit decl scope.
// e.g: to hide for-loop variables from using them outside loop scope
extern void csema_push_scope(csema* self);

extern void csema_push_switch_stmt_info(csema* self, tree_stmt* switch_stmt);
extern void csema_pop_switch_stmt_info(csema* self);
extern void csema_set_switch_stmt_has_default_label(csema* self);
extern cswitch_stmt_info* csema_get_switch_stmt_info(const csema* self);
extern bool csema_in_switch_stmt(const csema* self);
extern bool csema_switch_stmt_has_default_label(const csema* self);
extern bool csema_switch_stmt_register_case_label(const csema* self, tree_stmt* label);

extern bool csema_at_file_scope(const csema* self);
extern bool csema_at_block_scope(const csema* self);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_H
