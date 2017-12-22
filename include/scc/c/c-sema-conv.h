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
        csema* self, tree_expr** lhs, tree_expr** rhs, bool convert_lhs);

extern tree_type* csema_default_argument_promotion(csema* self, tree_expr** e);

typedef enum
{
        CACRK_COMPATIBLE,
        CACRK_INCOMPATIBLE,
        CACRK_RHS_NOT_AN_ARITHMETIC,
        CACRK_RHS_NOT_A_RECORD,
        CACRK_INCOMPATIBLE_RECORDS,
        CACRK_QUAL_DISCARTION,
        CACRK_INCOMPATIBLE_POINTERS,
} cassign_conv_result_kind;

typedef struct
{
        cassign_conv_result_kind kind;
        tree_type_quals discarded_quals;
} cassign_conv_result;

extern tree_type* csema_assignment_conversion(
        csema* self, tree_type* lt, tree_expr** rhs, cassign_conv_result* r);

#ifdef __cplusplus
}
#endif

#endif // !CSEMA_CONVERSIONS_H
