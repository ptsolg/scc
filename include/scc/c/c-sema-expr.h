#ifndef CSEMA_EXPR_H
#define CSEMA_EXPR_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern bool csema_require_object_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_function_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_integral_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_integer_expr(const csema* self, const tree_expr* e);

extern bool csema_require_real_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_record_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_array_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_scalar_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_scalar_expr(const csema* self, const tree_expr* e);

extern bool csema_require_arithmetic_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_real_or_object_pointer_expr_type(
        const csema* self, const tree_type* t, tree_location l);

extern bool csema_require_lvalue_or_function_designator(
        const csema* self, const tree_expr* e);

extern bool csema_require_modifiable_lvalue(const csema* self, const tree_expr* e);

extern bool csema_require_compatible_expr_types(
        const csema* self, const tree_type* a, const tree_type* b, tree_location l);

extern tree_expr* csema_new_paren_expr(csema* self, tree_location loc, tree_expr* expr);
extern tree_expr* csema_new_decl_expr(csema* self, tree_location loc, tree_id name);

extern tree_expr* csema_new_sp_floating_literal(csema* self, tree_location loc, float v);
extern tree_expr* csema_new_dp_floating_literal(csema* self, tree_location loc, ldouble v);
extern tree_expr* csema_new_character_literal(csema* self, tree_location loc, int c);
extern tree_expr* csema_new_integer_literal(
        csema* self, tree_location loc, suint64 v, bool signed_, bool ext);
extern tree_expr* csema_new_string_literal(csema* self, tree_location loc, tree_id ref);

extern tree_expr* csema_new_subscript_expr(
        csema* self, tree_location loc, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* csema_new_call_expr(
        csema* self, tree_location loc, tree_expr* lhs, dseq* args);

extern tree_expr* csema_new_member_expr(
        csema* self,
        tree_location loc,
        tree_expr* lhs,
        tree_id id,
        tree_location id_loc,
        bool is_arrow);

extern tree_expr* csema_new_unary_expr(
        csema* self, tree_location loc, tree_unop_kind opcode, tree_expr* expr);

extern tree_expr* csema_new_sizeof_expr(csema* self, tree_location loc, csizeof_rhs* rhs);

extern tree_expr* csema_new_cast_expr(
        csema* self, tree_location loc, tree_type* type, tree_expr* expr);

extern tree_expr* csema_new_binary_expr(
        csema* self, tree_location loc, tree_binop_kind opcode, tree_expr* lhs, tree_expr* rhs);

extern tree_expr* csema_new_conditional_expr(
        csema* self,
        tree_location loc,
        tree_expr* condition,
        tree_expr* lhs,
        tree_expr* rhs);

extern tree_designation* csema_new_designation(csema* self);

extern tree_designation* csema_finish_designation(
        csema* self, tree_expr* list, tree_designation* d, tree_expr* init);

// adds empty designation to initializer list to check whether it has trailing comma or not
extern bool csema_add_empty_designation(csema* self, tree_expr* initializer_list);

extern tree_type* csema_get_designation_type(csema* self, tree_designation* d);

extern tree_designator* csema_new_member_designator(
        csema* self, tree_location loc, tree_type* t, tree_id name);

extern tree_designator* csema_new_array_designator(
        csema* self, tree_location loc, tree_type* t, tree_expr* index);

extern tree_designator* csema_finish_designator(
        csema* self, tree_designation* designation, tree_designator* designator);

extern tree_type* csema_get_designator_type(csema* self, tree_designator* d);

extern tree_expr* csema_new_init_expr(csema* self, tree_location loc);

extern tree_expr* csema_finish_expr(csema* self, tree_expr* expr);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_EXPR_H
