#ifndef TREE_EVAL_H
#define TREE_EVAL_H

#ifdef S_HAS_PRAGMA
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif
 
#include "tree-common.h"

typedef struct _tree_exp         tree_exp;
typedef struct _tree_type        tree_type;
typedef struct _tree_target_info tree_target_info;

typedef struct _tree_eval_result
{
        const tree_type* _type;
        union
        {
                suint64 _int;
                float   _float;
                ldouble _double;
        };
} tree_eval_result;

//extern void tree_eval_result_init(tree_eval_result* self, const tree_type* type);

extern bool tree_eval_exp(const tree_exp* e, const tree_target_info* i, tree_eval_result* r);

#ifdef __cplusplus
}
#endif

#endif // !TREE_EVAL_H