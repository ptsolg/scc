#ifndef C_ERRORS_H
#define C_ERRORS_H

#include "scc/lex/token.h"
#include "scc/tree/common.h"

typedef struct _tree_decl tree_decl;

extern void c_error_expected_a_before_b(
        c_context* self, tree_location loc, c_token_kind a, c_token_kind b);

extern void c_error_expected_a_or_b_before_c(
        c_context* self, tree_location loc, c_token_kind a, c_token_kind b, c_token_kind c);

extern void c_error_expected_one_of(
        c_context* self, tree_location loc, c_token_kind* v, size_t n, c_token_kind end);

extern void c_error_unknown_type_name(c_context* self, const c_token* id);
extern void c_error_expected_type_specifier(c_context* self, tree_location loc);
extern void c_error_invalid_type_specifier(c_context* self, tree_location loc);
extern void c_error_empty_struct(c_context* self, tree_location loc);
extern void c_error_empty_enum(c_context* self, tree_location loc);
extern void c_error_expected_expr(c_context* self, tree_location loc);
extern void c_error_empty_initializer(c_context* self, tree_location loc);
extern void c_error_function_initialized_like_a_variable_(c_context* self, const tree_decl* func);

#endif