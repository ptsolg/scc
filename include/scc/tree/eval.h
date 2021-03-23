#ifndef TREE_EVAL_H
#define TREE_EVAL_H

#include "scc/core/value.h"

typedef struct _tree_context tree_context;
typedef struct _tree_expr tree_expr;

typedef enum
{
        TERK_INVALID,
        TERK_INTEGER,
        TERK_FLOATING,
        TERK_ADDRESS_CONSTANT,
} tree_eval_result_kind;

typedef struct _tree_eval_result
{
        tree_eval_result_kind kind;
        avalue value;
        const tree_expr* error;
} tree_eval_result;

extern bool tree_eval_expr(
        tree_context* context, const tree_expr* expr, tree_eval_result* result);

extern bool tree_eval_expr_as_integer(
        tree_context* context, const tree_expr* expr, tree_eval_result* result);

extern bool tree_eval_expr_as_arithmetic(
        tree_context* context, const tree_expr* expr, tree_eval_result* result);

#endif // !TREE_EVAL_H
