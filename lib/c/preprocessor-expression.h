#ifndef C_PREPROCESSOR_EXPRESSION_H
#define C_PREPROCESSOR_EXPRESSION_H

#include "scc/core/common.h"

typedef struct _c_preprocessor c_preprocessor;
typedef struct _int_value int_value;
typedef struct _c_token c_token;

// returns token after expression
extern c_token* c_preprocessor_evaluate_expr(c_preprocessor* self, int_value* result);

#endif