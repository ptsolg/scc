#ifndef CERRORS_H
#define CERRORS_H

#include "scc/tree/tree-common.h"
#include "scc/c/c-token.h"

typedef struct _clogger clogger;
typedef struct _ctoken ctoken;
typedef struct _cdecl_specs cdecl_specs;
typedef struct _cdeclarator cdeclarator;
typedef struct _tree_context tree_context;
typedef struct _tree_decl tree_decl;
typedef struct _tree_expr tree_expr;
typedef struct _tree_stmt tree_stmt;

// preprocessing lexer
extern void cerror_cannot_open_source_file(clogger* self, tree_location loc, const char* file);
extern void cerror_token_is_too_long(clogger* self, tree_location loc);
extern void cerror_missing_closing_quote(clogger* self, tree_location loc);
extern void cerror_empty_character_constant(clogger* self, tree_location loc);
extern void cerror_invalid_character_constant(clogger* self, tree_location loc);
extern void cerror_unclosed_comment(clogger* self, tree_location loc);
extern void cerror_unknown_punctuator(clogger* self, tree_location loc, int c);
extern void cerror_unknown_symbol(clogger* self, tree_location loc, int c);

// preprocessor
extern void cerror_too_deep_include_nesting(clogger* self);
extern void cerror_expected_file_name(clogger* self, tree_location loc);
extern void cerror_empty_file_name_in_include(clogger* self, tree_location loc);
extern void cerror_cannot_open_source_file(clogger* self, tree_location loc, const char* file);
extern void cerror_unexpected_hash(clogger* self, tree_location loc);
extern void cerror_unknown_preprocessor_directive(clogger* self, tree_location loc);
extern void cerror_unsupported_preprocessor_directive(clogger* self, tree_location loc);

// lexer
extern void cerror_invalid_integer_literal(clogger* self, tree_location loc, const char* num);
extern void cerror_invalid_floating_literal(clogger* self, tree_location loc, const char* num);

// parser
extern void cerror_expected_a_before_b(
        clogger* self, tree_location loc, ctoken_kind a, ctoken_kind b);

extern void cerror_expected_a_or_b_before_c(
        clogger* self, tree_location loc, ctoken_kind a, ctoken_kind b, ctoken_kind c);

extern void cerror_expected_one_of(
        clogger* self, tree_location loc, ctoken_kind* v, ssize n, ctoken_kind end);

extern void cerror_unknown_type_name(clogger* self, const ctoken* id);
extern void cerror_expected_type_specifier(clogger* self, tree_location loc);
extern void cerror_invalid_type_specifier(clogger* self, tree_location loc);
extern void cerror_empty_struct(clogger* self, tree_location loc);
extern void cerror_empty_enum(clogger* self, tree_location loc);
extern void cerror_expected_expr(clogger* self, tree_location loc);
extern void cerror_empty_initializer(clogger* self, tree_location loc);
extern void cerror_too_many_initializer_values(clogger* self, tree_location loc);

// sema
extern void cerror_undeclared_identifier(clogger* self, tree_location loc, tree_id name);
extern void cerror_multiple_storage_classes(clogger* self, const cdecl_specs* ds);
extern void cerror_inline_allowed_on_functions_only(clogger* self, tree_location loc);
extern void cerror_invalid_parameter_storage_class(clogger* self, const cdeclarator* d);

extern void cerror_function_initialized_like_a_variable(clogger* self, const tree_decl* func);

extern void cerror_named_argument_before_ellipsis_required(clogger* self, tree_location loc);
extern void cerror_redefinition(clogger* self, tree_location loc, tree_id id);
extern void cerror_decl_redefinition(clogger* self, const tree_decl* decl);
extern void cerror_enumerator_value_isnt_constant(
        clogger* self, tree_location loc, tree_id name);

extern void cerror_wrong_king_of_tag(clogger* self, tree_location loc, tree_id name);
extern void cerror_field_function(clogger* self, const tree_decl* field);
extern void cerror_invalid_bitfield_type(clogger* self, const tree_decl* field);
extern void cerror_bitfield_width_isnt_constant(clogger* self, const tree_decl* field);
extern void cerror_bitfield_width_is_zero(clogger* self, const tree_decl* field);
extern void cerror_negative_bitfield_width(clogger* self, const tree_decl* field);
extern void cerror_bitfield_width_exceeds_type(clogger* self, const tree_decl* field);
extern void cerror_invalid_storage_class(clogger* self, const tree_decl* decl);
extern void cerror_different_kind_of_symbol(clogger* self, const tree_decl* decl);
extern void cerror_different_storage_class(clogger* self, const tree_decl* decl);
extern void cerror_conflicting_types(clogger* self, const tree_decl* decl);
extern void cerror_parameter_name_omitted(clogger* self, const tree_decl* param);
extern void cerror_function_isnt_allowed_here(clogger* self, const tree_decl* func);

extern void cerror_expr_must_have_pointer_to_object_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_pointer_to_function_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_integral_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_real_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_record_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_array_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_scalar_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_arithmetic_type(clogger* self, tree_location loc);
extern void cerror_expr_must_have_real_or_pointer_to_object_type(clogger* self, tree_location loc);
extern void cerror_expr_must_be_lvalue_or_function_designator(clogger* self, const tree_expr* e);
extern void cerror_expr_must_be_modifiable_lvalue(clogger* self, const tree_expr* e);
extern void cerror_types_are_not_compatible(clogger* self, tree_location loc);
extern void cerror_subscripted_value_isnt_array(clogger* self, tree_location loc);
extern void cerror_incompatible_type_for_argument(clogger* self, tree_location loc, unsigned pos);
extern void cerror_passing_argument_discards_qualifer(
        clogger* self, tree_location loc, unsigned pos, int quals);

extern void cerror_passing_argument_from_incompatible_pointer_type(
        clogger* self, tree_location loc, unsigned pos);

extern void cerror_to_many_arguments(clogger* self, const tree_expr* call);
extern void cerror_to_few_arguments(clogger* self, const tree_expr* call);
extern void cerror_operand_of_sizeof_is_function(clogger* self, tree_location loc);
extern void cerror_operand_of_sizeof_is_bitfield(clogger* self, tree_location loc);
extern void cerror_invalid_binop_operands(clogger* self, tree_location loc, int opcode);
extern void cerror_cmp_of_distinct_pointers(clogger* self, tree_location loc);
extern void cerror_assignment_discards_quals(clogger* self, tree_location loc, int quals);
extern void cerror_assignment_from_incompatible_pointer_type(clogger* self, tree_location loc);
extern void cerror_pointer_type_mismatch(clogger* self, tree_location loc);
extern void cerror_type_mismatch(clogger* self, tree_location loc);

extern void cerror_case_stmt_outside_switch(clogger* self, tree_location loc);
extern void cerror_case_stmt_isnt_constant(clogger* self, tree_location loc);
extern void cerror_case_stmt_duplication(clogger* self, tree_location loc);
extern void cerror_default_stmt_outside_switch(clogger* self, tree_location loc);
extern void cerror_default_stmt_duplication(clogger* self, tree_location loc);
extern void cerror_non_variable_decl_in_for_loop(clogger* self, const tree_decl* decl);
extern void cerror_invalid_storage_class_for_loop_decl(clogger* self, const tree_decl* decl);
extern void cerror_return_non_void(clogger* self, const tree_expr* expr);
extern void cerror_return_type_doesnt_match(clogger* self, const tree_expr* expr);
extern void cerror_return_discards_quals(clogger* self, tree_location loc, int quals);
extern void cerror_return_from_incompatible_pointer_type(clogger* self, tree_location loc);
extern void cerror_break_stmt_outside_loop_or_switch(clogger* self, const tree_stmt* stmt);
extern void cerror_continue_stmt_outside_loop(clogger* self, const tree_stmt* stmt);
extern void cerror_decl_stmt_outside_block(clogger* self, const tree_stmt* stmt);
extern void cerror_undefined_label(clogger* self, const tree_decl* label);

extern void cerror_incomplete_type(clogger* self, tree_location loc);
extern void cerror_array_of_functions(clogger* self, tree_location loc);
extern void cerror_array_size_isnt_integer(clogger* self, tree_location loc);
extern void cerror_array_must_be_greater_than_zero(clogger* self, tree_location loc);
extern void cerror_function_returning_array(clogger* self, tree_location loc);
extern void cerror_function_returning_function(clogger* self, tree_location loc);
extern void cerror_invalid_use_of_restrict(clogger* self, tree_location loc);

#endif