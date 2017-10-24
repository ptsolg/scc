#ifndef CPROG_CONVERSIONS_H
#define CPROG_CONVERSIONS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern tree_exp*  cprog_build_impl_cast(cprog* self, tree_exp* e, tree_type* t);
extern tree_type* cprog_perform_lvalue_conversion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_array_to_pointer_conversion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_function_to_pointer_conversion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_integer_promotion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_array_function_to_pointer_conversion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_unary_conversion(cprog* self, tree_exp** e);
extern tree_type* cprog_perform_usual_arithmetic_conversion(
        cprog* self, tree_exp** lhs, tree_exp** rhs);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_CONVERSIONS_H