#ifndef C_PREPROCESSOR_DIRECTIVE_H
#define C_PREPROCESSOR_DIRECTIVE_H

#include "scc/core/common.h"
#include "scc/tree/tree-common.h"

typedef struct _c_preprocessor c_preprocessor;
typedef struct _c_token c_token;

extern bool c_preprocessor_handle_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_if_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_ifdef_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_ifndef_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_elif_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_else_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_endif_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_include_directive(c_preprocessor* self);
extern bool c_preprocessor_handle_define_directive(c_preprocessor* self);
extern bool c_preprocessor_handle_error_directive(c_preprocessor* self, c_token* tok);
extern bool c_preprocessor_handle_pragma_directive(c_preprocessor* self);

#endif