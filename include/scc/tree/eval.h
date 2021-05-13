#ifndef TREE_EVAL_H
#define TREE_EVAL_H

#include "scc/core/num.h"

typedef struct _tree_target_info tree_target_info;
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
        struct num value;
        const tree_expr* error;
} tree_eval_result;

extern bool tree_eval_expr(
        const tree_target_info* target, const tree_expr* expr, tree_eval_result* result);

extern bool tree_eval_expr_as_integer(
        const tree_target_info* target, const tree_expr* expr, tree_eval_result* result);

extern bool tree_eval_expr_as_arithmetic(
        const tree_target_info* target, const tree_expr* expr, tree_eval_result* result);

#endif // !TREE_EVAL_H
