#ifndef C_PREPROCESSOR_EXPRESSION_H
#define C_PREPROCESSOR_EXPRESSION_H

#include "scc/core/common.h"

typedef struct _c_preprocessor c_preprocessor;
typedef struct _c_token c_token;
struct num;

// returns token after expression
extern c_token* c_preprocessor_evaluate_expr(c_preprocessor* self, struct num* result);

#endif
