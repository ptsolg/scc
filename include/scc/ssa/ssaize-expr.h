#ifndef SSAIZE_EXPR_H
#define SSAIZE_EXPR_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "ssaizer.h"

extern ssa_value* ssaize_binary_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_unary_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_call_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_subscript_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_conditional_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_integer_literal(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_character_literal(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_floating_literal(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_string_literal(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_decl_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_member_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_cast_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_sizeof_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_init_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_expr(ssaizer* self, const tree_expr* expr);
extern ssa_value* ssaize_expr_as_condition(ssaizer* self, const tree_expr* expr);

#ifdef __cplusplus
}
#endif

#endif