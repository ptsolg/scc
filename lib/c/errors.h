#ifndef C_ERRORS_H
#define C_ERRORS_H

#include "scc/tree/common.h"
#include "scc/c/token.h"

typedef struct _c_token c_token;
typedef struct _c_macro c_macro;
typedef struct _c_decl_specs c_decl_specs;
typedef struct _c_declarator c_declarator;
typedef struct _tree_context tree_context;
typedef struct _tree_decl tree_decl;
typedef struct _tree_expr tree_expr;
typedef struct _tree_stmt tree_stmt;
typedef struct _tree_designator tree_designator;
typedef struct _c_assignment_conversion_result c_assignment_conversion_result;

// token lexer
extern void c_error_cannot_open_source_file(c_context* self, tree_location loc, const char* file);
extern void c_error_token_is_too_long(c_context* self, tree_location loc);
extern void c_error_missing_closing_quote(c_context* self, tree_location loc);
extern void c_error_empty_character_constant(c_context* self, tree_location loc);
extern void c_error_invalid_character_constant(c_context* self, tree_location loc);
extern void c_error_unclosed_comment(c_context* self, tree_location loc);
extern void c_error_stray_symbol(c_context* self, tree_location loc, int c);

// preprocessor
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

// lexer
extern void c_error_invalid_integer_literal(c_context* self, tree_location loc, const char* num);
extern void c_error_invalid_floating_literal(c_context* self, tree_location loc, const char* num);

// parser
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
extern void c_error_too_many_initializer_values(c_context* self, tree_location loc);

// sema
extern void c_error_undeclared_identifier(c_context* self, tree_location loc, tree_id name);
extern void c_error_multiple_storage_classes(c_context* self, const c_decl_specs* ds);
extern void c_error_invalid_specifier(c_context* self, tree_location loc, c_token_kind spec, int allowed_decl);
extern void c_error_invalid_parameter_storage_class(c_context* self, const c_declarator* d);
extern void c_error_variable_declared_thread_local_at_function_scope(c_context* self, const c_declarator* d);
extern void c_error_variable_declared_register_at_file_scope(c_context* self, const c_declarator* d);
extern void c_error_field_declared_with_storage_specifier(c_context* self, const c_declarator* d);
extern void c_error_dllimport_cannot_be_applied_to_definition(c_context* self, const c_decl_specs* specs, int decl_kind);
extern void c_error_dllimport_applied_to_wrong_decl(c_context* self, tree_location loc);
extern void c_error_extern_variable_has_an_initializer(c_context* self, const c_declarator* d);

extern void c_error_function_initialized_like_a_variable(c_context* self, const tree_decl* func);
extern void c_error_invalid_initializer(c_context* self, const tree_expr* init);
extern void c_error_initialization_discards_qualifer(c_context* self, const tree_expr* init, int quals);
extern void c_error_initialization_from_incompatible_pointer_types(c_context* self, const tree_expr* init);
extern void c_error_braces_around_scalar_initializer(c_context* self, tree_location loc);
extern void c_error_field_name_not_in_record_initializer(c_context* self, const tree_designator* d);
extern void c_error_array_index_in_non_array_intializer(c_context* self, const tree_designator* d);
extern void c_error_array_index_in_initializer_not_of_integer_type(c_context* self, const tree_expr* index);
extern void c_error_nonconstant_array_index_in_initializer(c_context* self, const tree_expr* index);
extern void c_error_array_index_in_initializer_exceeds_array_bounds(c_context* self, const tree_expr* index);
extern void c_error_initializer_element_isnt_constant(c_context* self, const tree_expr* init);
extern void c_error_initializer_string_is_too_long(c_context* self, const tree_expr* init);
extern void c_error_wide_character_array_initialized_from_non_wide_string(c_context* self, const tree_expr* init);

extern void c_error_named_argument_before_ellipsis_required(c_context* self, tree_location loc);
extern void c_error_redefinition(c_context* self, tree_location loc, tree_id id);
extern void c_error_decl_redefinition(c_context* self, const tree_decl* decl);
extern void c_error_enumerator_value_isnt_constant(c_context* self, tree_location loc, tree_id name);

extern void c_error_wrong_king_of_tag(c_context* self, tree_location loc, tree_id name);
extern void c_error_field_function(c_context* self, const tree_decl* field);
extern void c_error_invalid_bitfield_type(c_context* self, const tree_decl* field);
extern void c_error_bitfield_width_isnt_constant(c_context* self, const tree_decl* field);
extern void c_error_bitfield_width_is_zero(c_context* self, const tree_decl* field);
extern void c_error_negative_bitfield_width(c_context* self, const tree_decl* field);
extern void c_error_bitfield_width_exceeds_type(c_context* self, const tree_decl* field);
extern void c_error_invalid_storage_class(c_context* self, tree_location loc, tree_id name);
extern void c_error_different_kind_of_symbol(c_context* self, const tree_decl* decl);
extern void c_error_different_storage_class(c_context* self, const tree_decl* decl);
extern void c_error_conflicting_types(c_context* self, const tree_decl* decl);
extern void c_error_parameter_name_omitted(c_context* self, tree_location loc);
extern void c_error_function_isnt_allowed_here(c_context* self, tree_location loc);

extern void c_error_expr_must_have_pointer_to_object_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_pointer_to_function_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_integral_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_real_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_record_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_array_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_scalar_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_arithmetic_type(c_context* self, tree_location loc);
extern void c_error_expr_must_have_real_or_pointer_to_object_type(c_context* self, tree_location loc);
extern void c_error_expr_must_be_lvalue_or_function_designator(c_context* self, const tree_expr* e);
extern void c_error_expr_must_be_modifiable_lvalue(c_context* self, const tree_expr* e);
extern void c_error_types_are_not_compatible(c_context* self, tree_location loc);
extern void c_error_subscripted_value_isnt_array(c_context* self, tree_location loc);
extern void c_error_incompatible_type_for_argument(c_context* self, tree_location loc, unsigned pos);
extern void c_error_passing_argument_discards_qualifer(
        c_context* self, tree_location loc, unsigned pos, int quals);

extern void c_error_passing_argument_from_incompatible_pointer_type(
        c_context* self, tree_location loc, unsigned pos);

extern void c_error_invalid_scalar_initialization(
        c_context* self, const tree_expr* e, const c_assignment_conversion_result* r);
extern void c_error_invalid_argument_assignment(
        c_context* self, tree_location loc, unsigned pos, const c_assignment_conversion_result* r);
extern void c_error_invalid_assignment(c_context* self, tree_location op_loc, 
        const tree_expr* rhs, const c_assignment_conversion_result* r);
extern void c_error_invalid_return_type(c_context* self, const tree_expr* e, const c_assignment_conversion_result* r);

extern void c_error_too_many_arguments(c_context* self, const tree_expr* call);
extern void c_error_too_few_arguments(c_context* self, const tree_expr* call);
extern void c_error_operand_of_sizeof_is_function(c_context* self, tree_location loc);
extern void c_error_operand_of_sizeof_is_bitfield(c_context* self, tree_location loc);
extern void c_error_invalid_binop_operands(c_context* self, tree_location loc, int opcode);
extern void c_error_cmp_of_distinct_pointers(c_context* self, tree_location loc);
extern void c_error_assignment_discards_quals(c_context* self, tree_location loc, int quals);
extern void c_error_assignment_from_incompatible_pointer_type(c_context* self, tree_location loc);
extern void c_error_pointer_type_mismatch(c_context* self, tree_location loc);
extern void c_error_type_mismatch(c_context* self, tree_location loc);

extern void c_error_case_stmt_outside_switch(c_context* self, tree_location loc);
extern void c_error_case_stmt_isnt_constant(c_context* self, tree_location loc);
extern void c_error_case_stmt_duplication(c_context* self, tree_location loc);
extern void c_error_default_stmt_outside_switch(c_context* self, tree_location loc);
extern void c_error_default_stmt_duplication(c_context* self, tree_location loc);
extern void c_error_non_variable_decl_in_for_loop(c_context* self, const tree_decl* decl);
extern void c_error_invalid_storage_class_for_loop_decl(c_context* self, const tree_decl* decl);
extern void c_error_return_non_void(c_context* self, const tree_expr* expr);
extern void c_error_return_type_doesnt_match(c_context* self, const tree_expr* expr);
extern void c_error_return_discards_quals(c_context* self, tree_location loc, int quals);
extern void c_error_return_from_incompatible_pointer_type(c_context* self, tree_location loc);
extern void c_error_break_stmt_outside_loop_or_switch(c_context* self, const tree_stmt* stmt);
extern void c_error_continue_stmt_outside_loop(c_context* self, const tree_stmt* stmt);
extern void c_error_decl_stmt_outside_block(c_context* self, const tree_stmt* stmt);
extern void c_error_undefined_label(c_context* self, const tree_decl* label);

extern void c_error_incomplete_type(c_context* self, tree_location loc);
extern void c_error_array_of_functions(c_context* self, tree_location loc);
extern void c_error_array_size_isnt_integer(c_context* self, tree_location loc);
extern void c_error_array_must_be_greater_than_zero(c_context* self, tree_location loc);
extern void c_error_function_returning_array(c_context* self, tree_location loc);
extern void c_error_function_returning_function(c_context* self, tree_location loc);
extern void c_error_invalid_use_of_restrict(c_context* self, tree_location loc);

extern void c_error_reffering_volatile_object_is_not_allowed(c_context* self, tree_location loc, bool in_atomic_block);
extern void c_error_transaction_unsafe_function_is_not_allowed(c_context* self, tree_location loc, bool in_atomic_block);
extern void c_error_volatile_param_is_not_allowed(c_context* self, tree_location loc);
extern void c_error_jumping_into_the_atomic_block_is_prohibited(c_context* self, tree_location loc, c_token_kind stmt_kind);

#endif