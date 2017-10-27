#ifndef CPROG_EXP_H
#define CPROG_EXP_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern bool cprog_require_object_pointer_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_function_pointer_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_integral_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_real_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_record_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_array_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_scalar_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_arithmetic_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_real_or_object_pointer_exp_type(
        const cprog* self, const tree_type* t, tree_location l);

extern bool cprog_require_lvalue_or_function_designator(
        const cprog* self, const tree_exp* e);

extern bool cprog_require_modifiable_lvalue(const cprog* self, const tree_exp* e);

extern bool cprog_require_compatible_exp_types(
        const cprog* self, const tree_type* a, const tree_type* b, tree_location l);

extern tree_exp* cprog_build_paren_exp(cprog* self, tree_location loc, tree_exp* exp);
extern tree_exp* cprog_build_decl_exp(cprog* self, tree_location loc, tree_id name);

extern tree_exp* cprog_build_floating_literal(cprog* self, tree_location loc, float v);
extern tree_exp* cprog_build_floating_lliteral(cprog* self, tree_location loc, ldouble v);
extern tree_exp* cprog_build_character_literal(cprog* self, tree_location loc, int c);
extern tree_exp* cprog_build_integer_literal(
        cprog* self, tree_location loc, suint64 v, bool signed_, bool ext);
extern tree_exp* cprog_build_string_literal(cprog* self, tree_location loc, tree_id ref);

extern tree_exp* cprog_build_subscript_exp(
        cprog* self, tree_location loc, tree_exp* lhs, tree_exp* rhs);

extern tree_exp* cprog_build_call_exp(
        cprog* self, tree_location loc, tree_exp* lhs, objgroup* args);

extern tree_exp* cprog_build_member_exp(
        cprog*        self,
        tree_location loc,
        tree_exp*     lhs,
        tree_id       id,
        tree_location id_loc,
        bool          is_arrow);

extern tree_exp* cprog_build_unop(
        cprog* self, tree_location loc, tree_unop_kind opcode, tree_exp* exp);

extern tree_exp* cprog_build_sizeof(cprog* self, tree_location loc, csizeof_rhs* rhs);

extern tree_exp* cprog_build_cast(
        cprog* self, tree_location loc, tree_type* type, tree_exp* exp);

extern tree_exp* cprog_build_binop(
        cprog* self, tree_location loc, tree_binop_kind opcode, tree_exp* lhs, tree_exp* rhs);

extern tree_exp* cprog_build_conditional(
        cprog*        self,
        tree_location loc,
        tree_exp*     condition,
        tree_exp*     lhs,
        tree_exp*     rhs);

extern tree_const_exp*   cprog_build_const_exp(cprog* self, tree_exp* root);
extern tree_designation* cprog_build_designation(cprog* self);

extern tree_designation* cprog_finish_designation(
        cprog*            self,
        tree_exp*         initializer_list,
        tree_designation* designation,
        tree_exp*         designation_initializer);

// adds empty designation to initializer list to check whether it has trailing comma or not
extern bool cprog_add_empty_designation(cprog* self, tree_exp* initializer_list);

extern tree_type* cprog_get_designation_type(cprog* self, tree_designation* d);

extern tree_designator* cprog_build_member_designator(
        cprog* self, tree_location loc, tree_type* t, tree_id name);

extern tree_designator* cprog_build_array_designator(
        cprog* self, tree_location loc, tree_type* t, tree_const_exp* index);

extern tree_designator* cprog_finish_designator(
        cprog* self, tree_designation* designation, tree_designator* designator);

extern tree_type* cprog_get_designator_type(cprog* self, tree_designator* d);

extern tree_exp* cprog_build_init_exp(cprog* self, tree_location loc);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_EXP_H