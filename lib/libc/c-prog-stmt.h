#ifndef CPROG_STMT_H
#define CPROG_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern tree_stmt* cprog_finish_stmt(cprog* self, tree_stmt* s);

extern tree_stmt* cprog_build_block_stmt(
        cprog* self, tree_location lbrace_loc, cstmt_context context);

extern tree_stmt* cprog_build_case_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location colon_loc,
        tree_exp* value,
        tree_stmt* body);

extern tree_stmt* cprog_build_default_stmt(
        cprog* self, tree_location kw_loc, tree_location colon_loc, tree_stmt* body);

extern tree_stmt* cprog_build_labeled_stmt(
        cprog* self,
        tree_location id_loc,
        tree_location colon_loc,
        tree_id name,
        tree_stmt* target);

extern tree_stmt* cprog_build_exp_stmt(
        cprog* self, tree_location begin_loc, tree_location semicolon_loc, tree_exp* exp);

extern tree_stmt* cprog_build_if_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* condition,
        tree_stmt* body,
        tree_stmt* else_);

extern tree_stmt* cprog_build_decl_stmt(
        cprog* self,tree_location begin_loc, tree_location semicolon_loc, tree_decl* d);

extern tree_stmt* cprog_build_switch_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* value,
        tree_stmt* body);

extern tree_stmt* cprog_build_while_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_exp* condition,
        tree_stmt* body);

extern tree_stmt* cprog_build_do_while_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location semicolon_loc,
        tree_exp* condition,
        tree_stmt* body);

extern tree_stmt* cprog_build_for_stmt(
        cprog* self,
        tree_location kw_loc,
        tree_location rbracket_loc,
        tree_stmt* init,
        tree_exp* condition, 
        tree_exp* step,
        tree_stmt* body);

extern tree_stmt* cprog_build_goto_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc, tree_id label);

extern tree_stmt* cprog_build_continue_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* cprog_build_break_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc);

extern tree_stmt* cprog_build_return_stmt(
        cprog* self, tree_location kw_loc, tree_location semicolon_loc, tree_exp* value);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_STMT_H
