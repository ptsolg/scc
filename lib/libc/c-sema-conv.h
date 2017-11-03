#ifndef CSEMA_CONVERSIONS_H
#define CSEMA_CONVERSIONS_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"

extern tree_expr* csema_new_impl_cast(csema* self, tree_expr* e, tree_type* t);
extern tree_type* csema_lvalue_conversion(csema* self, tree_expr** e);
extern tree_type* csema_array_to_pointer_conversion(csema* self, tree_expr** e);
extern tree_type* csema_function_to_pointer_conversion(csema* self, tree_expr** e);
extern tree_type* csema_integer_promotion(csema* self, tree_expr** e);
extern tree_type* csema_array_function_to_pointer_conversion(csema* self, tree_expr** e);
extern tree_type* csema_unary_conversion(csema* self, tree_expr** e);
extern tree_type* csema_usual_arithmetic_conversion(
        csema* self, tree_expr** lhs, tree_expr** rhs);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_CONVERSIONS_H
