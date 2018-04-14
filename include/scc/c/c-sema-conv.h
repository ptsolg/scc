#ifndef C_SEMA_CONVERSIONS_H
#define C_SEMA_CONVERSIONS_H

#ifdef HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "c-sema.h"
#include "scc/tree/tree-type.h"

extern tree_expr* c_sema_new_impl_cast(c_sema* self, tree_expr* e, tree_type* t);
extern tree_type* c_sema_lvalue_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_array_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_function_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_integer_promotion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_array_function_to_pointer_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_unary_conversion(c_sema* self, tree_expr** e);
extern tree_type* c_sema_usual_arithmetic_conversion(
        c_sema* self, tree_expr** lhs, tree_expr** rhs, bool convert_lhs);

extern tree_type* c_sema_default_argument_promotion(c_sema* self, tree_expr** e);

typedef enum
{
        CACRK_COMPATIBLE,
        CACRK_INCOMPATIBLE,
        CACRK_RHS_NOT_AN_ARITHMETIC,
        CACRK_RHS_NOT_A_RECORD,
        CACRK_RHS_TRANSACTION_UNSAFE,
        CACRK_INCOMPATIBLE_RECORDS,
        CACRK_QUAL_DISCARTION,
        CACRK_INCOMPATIBLE_POINTERS,
} c_assignment_conversion_result_kind;

typedef struct _c_assignment_conversion_result
{
        c_assignment_conversion_result_kind kind;
        tree_type_quals discarded_quals;
} c_assignment_conversion_result;

extern tree_type* c_sema_assignment_conversion(
        c_sema* self, tree_type* lt, tree_expr** rhs, c_assignment_conversion_result* r);

#ifdef __cplusplus
}
#endif

#endif
