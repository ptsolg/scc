#ifndef C_SEMA_EXPR_H
#define C_SEMA_EXPR_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "sema.h"
#include "scc/tree/expr.h"

extern bool c_sema_require_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_function_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_integral_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_integer_expr(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_real_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_record_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_array_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_scalar_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_scalar_expr(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_arithmetic_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_real_or_object_pointer_expr_type(
        const c_sema* self, const tree_type* t, tree_location l);

extern bool c_sema_require_lvalue_or_function_designator(
        const c_sema* self, const tree_expr* e);

extern bool c_sema_require_modifiable_lvalue(const c_sema* self, const tree_expr* e);

extern bool c_sema_require_compatible_expr_types(
        const c_sema* self, const tree_type* a, const tree_type* b, tree_location l, bool ignore_modifiers);

extern tree_expr* c_sema_new_paren_expr(
        c_sema* self, tree_location lbracket_loc, tree_expr* expr, tree_location rbracket_loc);
extern tree_expr* c_sema_new_decl_expr(c_sema* self, tree_location loc, tree_id name);

extern tree_expr* c_sema_new_sp_floating_literal(c_sema* self, tree_location loc, float v);
extern tree_expr* c_sema_new_dp_floating_literal(c_sema* self, tree_location loc, ldouble v);
extern tree_expr* c_sema_new_character_literal(c_sema* self, tree_location loc, int c);
extern tree_expr* c_sema_new_integer_literal(
        c_sema* self, tree_location loc, uint64_t v, bool signed_, bool ext);
extern tree_expr* c_sema_new_string_literal(c_sema* self, tree_location loc, tree_id ref);

extern tree_expr* c_sema_new_subscript_expr(
        c_sema* self, tree_location loc, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* c_sema_new_call_expr(c_sema* self, tree_location loc, tree_expr* lhs);
extern void c_sema_add_call_expr_arg(c_sema* self, tree_expr* call, tree_expr* arg);
extern tree_expr* c_sema_check_call_expr_args(c_sema* self, tree_expr* call);

extern tree_expr* c_sema_new_member_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow);

extern tree_expr* c_sema_new_unary_expr(
        c_sema* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr);

extern tree_expr* c_sema_new_sizeof_expr(
        c_sema* self, tree_location loc, void* operand, bool contains_type);

extern tree_expr* c_sema_new_cast_expr(
        c_sema* self, tree_location loc, tree_type* type, tree_expr* expr);

extern tree_expr* c_sema_new_binary_expr(
        c_sema* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* c_sema_new_conditional_expr(
        c_sema* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs);

extern tree_expr* c_sema_finish_expr(c_sema* self, tree_expr* expr);

extern tree_expr* c_sema_new_initializer_list(c_sema* self, tree_location loc);
extern tree_expr* c_sema_add_initializer_list_expr(
        c_sema* self, tree_expr* list, tree_expr* expr);

extern tree_expr* c_sema_check_initializer(c_sema* self, tree_type* type, tree_expr* init);

extern tree_expr* c_sema_new_designation(c_sema* self);
extern tree_expr* c_sema_add_designation_designator(
        c_sema* self, tree_expr* designation, tree_designator* designator);
extern tree_expr* c_sema_set_designation_initializer(c_sema* self, tree_expr* designation, tree_expr* init);

extern tree_designator* c_sema_new_field_designator(c_sema* self, tree_location loc, tree_id field);
extern tree_designator* c_sema_new_array_designator(c_sema* self, tree_location loc, tree_expr* index);

#ifdef __cplusplus
}
#endif

#endif
