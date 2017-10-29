#ifndef TREE_EVAL_H
#define TREE_EVAL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "tree-target.h"
#include "tree-exp.h"
#include <libscl/value.h>

typedef struct _tree_eval_info
{
        const tree_target_info* _target;
        const tree_exp*         _error;
} tree_eval_info;

extern void tree_init_eval_info(tree_eval_info* self, const tree_target_info* target);
extern const tree_exp* tree_get_eval_eror(const tree_eval_info* self);

extern bool tree_eval_as_integer(tree_eval_info* info, const tree_exp* exp, int_value* result);
extern bool tree_eval_as_arithmetic(tree_eval_info* info, const tree_exp* exp, avalue* result);

#ifdef __cplusplus
}
#endif

#endif // !TREE_EVAL_H