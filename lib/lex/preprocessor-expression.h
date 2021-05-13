#ifndef C_PREPROCESSOR_EXPRESSION_H
#define C_PREPROCESSOR_EXPRESSION_H

#include "scc/core/common.h"
#include "scc/core/num.h"

typedef struct _c_preprocessor c_preprocessor;
typedef struct _c_token c_token;

// returns token after expression
extern c_token* c_preprocessor_evaluate_expr(c_preprocessor* self, struct num* result);

#endif
