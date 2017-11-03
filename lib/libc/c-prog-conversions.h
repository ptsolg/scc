#ifndef CPROG_CONVERSIONS_H
#define CPROG_CONVERSIONS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-prog.h"

extern tree_expr* cprog_build_impl_cast(cprog* self, tree_expr* e, tree_type* t);
extern tree_type* cprog_perform_lvalue_conversion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_array_to_pointer_conversion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_function_to_pointer_conversion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_integer_promotion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_array_function_to_pointer_conversion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_unary_conversion(cprog* self, tree_expr** e);
extern tree_type* cprog_perform_usual_arithmetic_conversion(
        cprog* self, tree_expr** lhs, tree_expr** rhs);

#ifdef __cplusplus
}
#endif

#endif // !CPROG_CONVERSIONS_H
