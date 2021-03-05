#ifndef C_ERRORS_H
#define C_ERRORS_H

#include "scc/c-common/context.h"
#include "scc/lex/token.h"

typedef struct _c_macro c_macro;

extern void c_error_cannot_open_source_file(c_context* self, tree_location loc, const char* file);
extern void c_error_token_is_too_long(c_context* self, tree_location loc);
extern void c_error_missing_closing_quote(c_context* self, tree_location loc);
extern void c_error_empty_character_constant(c_context* self, tree_location loc);
extern void c_error_invalid_character_constant(c_context* self, tree_location loc);
extern void c_error_unclosed_comment(c_context* self, tree_location loc);
extern void c_error_stray_symbol(c_context* self, tree_location loc, int c);

extern void c_error_too_deep_include_nesting(c_context* self);
extern void c_error_expected_file_name(c_context* self, tree_location loc);
extern void c_error_empty_file_name_in_include(c_context* self, tree_location loc);
extern void c_error_unknown_preprocessor_directive(c_context* self, tree_location loc);
extern void c_error_unsupported_preprocessor_directive(c_context* self, tree_location loc);
extern void c_error_no_macro_name_given_in_directive(c_context* self, c_token_kind directive, tree_location loc);
extern void c_error_macro_names_must_be_identifiers(c_context* self, tree_location loc);
extern void c_error_macro_redefenition(c_context* self, tree_id name, tree_location loc);
extern void c_error_expected_identifier(c_context* self, tree_location loc);
extern void c_error_missing_closing_bracket_in_macro_parameter_list(c_context* self, tree_location loc);
extern void c_error_macro_parameters_must_be_comma_separated(c_context* self, tree_location loc);
extern void c_error_hash2_cannot_appear_at_either_end_of_macro_expansion(c_context* self, tree_location loc);
extern void c_error_whitespace_after_macro_name_required(c_context* self, tree_location loc);
extern void c_error_invalid_pasting(c_context* self, c_token* a, c_token* b, tree_location loc);
extern void c_error_unterminated_macro_argument_list(c_context* self, c_macro* macro, tree_location loc);
extern void c_error_macro_argument_list_underflow(c_context* self, c_macro* macro, size_t args_given, tree_location loc);
extern void c_error_macro_argument_list_overflow(c_context* self, c_macro* macro, size_t args_given, tree_location loc);
extern void c_error_hash_is_not_followed_by_a_macro_param(c_context* self, tree_location loc);
extern void c_error_ending_directive_without_if(c_context* self, c_token* directive);
extern void c_error_extra_tokens_at_end_of_directive(c_context* self, c_token_kind directive, tree_location loc);
extern void c_error_missing_token_in_expression(c_context* self, c_token_kind k, tree_location l);
extern void c_error_floating_constant_in_preprocessor_expression(c_context* self, tree_location l);
extern void c_error_division_by_zero_in_preprocessor_expression(c_context* self, tree_location l);
extern void c_error_unterminated_directive(c_context* self, const c_token* directive);
extern void c_error_directive_after_else(c_context* self, const c_token* directive);
extern void c_error_token_is_not_valid_in_preprocessor_expressions(c_context* self, const c_token* tok);
extern void c_error_error_directive(c_context* self, tree_location loc, const char* msg);
extern void c_error_unknown_pragma(c_context* self, tree_location loc);
extern void c_error_expected_library_name(c_context* self, tree_location loc);
extern void c_error_empty_library_name(c_context* self, tree_location loc);
extern void c_error_expected_pp_expr(c_context* self, tree_location loc);

extern void c_error_invalid_integer_literal(c_context* self, tree_location loc, const char* num);
extern void c_error_invalid_floating_literal(c_context* self, tree_location loc, const char* num);

#endif