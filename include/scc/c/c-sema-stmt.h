#ifndef C_SEMA_STMT_H
#define C_SEMA_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern tree_stmt* c_sema_add_stmt(c_sema* self, tree_stmt* s);

extern tree_stmt* c_sema_new_block_stmt(
        c_sema* self, tree_location lbrace_loc, int scope_flags);

extern tree_stmt* c_sema_new_case_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr);

extern void c_sema_set_case_stmt_body(c_sema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* c_sema_new_default_stmt(
        c_sema* self, tree_location kw_loc, tree_location colon_loc);

extern void c_sema_set_default_stmt_body(c_sema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* c_sema_new_labeled_stmt(c_sema* self, tree_decl* label, tree_stmt* stmt);

extern tree_stmt* c_sema_new_expr_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr);

extern tree_stmt* c_sema_new_if_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_);

extern tree_stmt* c_sema_new_decl_stmt(
        c_sema* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d);

extern tree_stmt* c_sema_new_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        tree_stmt* body);

extern tree_stmt* c_sema_start_switch_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value);

extern tree_stmt* c_sema_finish_switch_stmt(c_sema* self, tree_stmt* switch_, tree_stmt* body);

extern tree_stmt* c_sema_new_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* c_sema_new_do_while_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* c_sema_new_for_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_expr* condition, 
        tree_expr* step,
        tree_stmt* body);

extern tree_stmt* c_sema_new_goto_stmt(
        c_sema* self,
        tree_location kw_loc,
        tree_location id_loc,
        tree_id id,
        tree_location semicolon_loc);

extern tree_stmt* c_sema_new_continue_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* c_sema_new_break_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* c_sema_new_return_stmt(
        c_sema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value);

extern bool c_sema_check_stmt(const c_sema* self, const tree_stmt* s, int scope_flags);

#ifdef __cplusplus
}
#endif

#endif
