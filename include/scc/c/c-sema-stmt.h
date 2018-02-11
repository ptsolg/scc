#ifndef CSEMA_STMT_H
#define CSEMA_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern tree_stmt* csema_add_stmt(csema* self, tree_stmt* s);

extern tree_stmt* csema_new_block_stmt(
        csema* self, tree_location lbrace_loc, int scope_flags);

extern tree_stmt* csema_new_case_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_expr* expr);

extern void csema_set_case_stmt_body(csema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* csema_new_default_stmt(
        csema* self, tree_location kw_loc, tree_location colon_loc);

extern void csema_set_default_stmt_body(csema* self, tree_stmt* stmt, tree_stmt* body);

extern tree_stmt* csema_new_labeled_stmt(csema* self, tree_decl* label, tree_stmt* stmt);

extern tree_stmt* csema_new_expr_stmt(
        csema* self, tree_location begin_loc, tree_location semicolon_loc, tree_expr* expr);

extern tree_stmt* csema_new_if_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body,
        tree_stmt* else_);

extern tree_stmt* csema_new_decl_stmt(
        csema* self, tree_location begin_loc, tree_location semicolon_loc, tree_decl* d);

extern tree_stmt* csema_new_switch_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value,
        tree_stmt* body);

extern tree_stmt* csema_start_switch_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* value);

extern tree_stmt* csema_finish_switch_stmt(csema* self, tree_stmt* switch_, tree_stmt* body);

extern tree_stmt* csema_new_while_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* csema_new_do_while_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_expr* condition,
        tree_stmt* body);

extern tree_stmt* csema_new_for_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_expr* condition, 
        tree_expr* step,
        tree_stmt* body);

extern tree_stmt* csema_new_goto_stmt(
        csema* self,
        tree_location kw_loc,
        tree_location id_loc,
        tree_id id,
        tree_location semicolon_loc);

extern tree_stmt* csema_new_continue_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* csema_new_break_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* csema_new_return_stmt(
        csema* self, tree_location kw_loc, tree_location semicolon_loc, tree_expr* value);

extern bool csema_check_stmt(const csema* self, const tree_stmt* s, int scope_flags);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_STMT_H
