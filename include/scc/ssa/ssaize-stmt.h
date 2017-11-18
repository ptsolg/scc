#ifndef SSAIZE_STMT_H
#define SSAIZE_STMT_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssaizer.h"

extern bool ssaize_labeled_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_default_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_case_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_compound_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_expr_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_if_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_switch_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_while_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_do_while_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_for_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_goto_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_continue_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_break_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_decl_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_return_stmt(ssaizer* self, const tree_stmt* stmt);
extern bool ssaize_stmt(ssaizer* self, const tree_stmt* stmt);

#ifdef __cplusplus
}
#endif

#endif