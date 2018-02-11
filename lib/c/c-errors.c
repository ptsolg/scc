#include "scc/c/c-errors.h"
#include "scc/c/c-error.h"
#include "scc/c/c-limits.h"
#include "scc/c/c-token.h"
#include "scc/c/c-info.h"
#include "scc/c/c-reswords.h"
#include "scc/c/c-sema-decl.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-stmt.h"

extern void cerror_cannot_open_source_file(clogger* self, tree_location loc, const char* file)
{
        cerror(self, CES_ERROR, loc, "cannot open source file \"%s\"", file);
}

extern void cerror_token_is_too_long(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "token overflowed internal buffer");
}

extern void cerror_missing_closing_quote(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "missing closing quote");
}

extern void cerror_empty_character_constant(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "empty character constant");
}

extern void cerror_invalid_character_constant(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "invalid character constant");
}

extern void cerror_unclosed_comment(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "comment unclosed at end of file");
}

extern void cerror_unknown_punctuator(clogger* self, tree_location loc, int c)
{
        cerror(self, CES_ERROR, loc, "unknown punctuator '%c'", c);
}

extern void cerror_unknown_symbol(clogger* self, tree_location loc, int c)
{
        cerror(self, CES_ERROR, loc, "unknown symbol '%c'", c);
}

extern void cerror_too_deep_include_nesting(clogger* self)
{
        cerror(self, CES_ERROR, TREE_INVALID_LOC,
                "maximum include nesting is %d", CMAX_INCLUDE_NESTING);
}

extern void cerror_expected_file_name(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expected a file name");
}

extern void cerror_empty_file_name_in_include(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "empty file name in #include");
}

extern void cerror_unexpected_hash(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "'#' is not expected here");
}

extern void cerror_unknown_preprocessor_directive(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "unknown preprocessing directive");
}

extern void cerror_unsupported_preprocessor_directive(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "unsupported preprocessor directive");
}

extern void cerror_invalid_integer_literal(clogger* self, tree_location loc, const char* num)
{
        cerror(self, CES_ERROR, loc, "invalid integer literal '%s'", num);
}

extern void cerror_invalid_floating_literal(clogger* self, tree_location loc, const char* num)
{
        cerror(self, CES_ERROR, loc, "invalid floating literal '%s'", num);
}

extern void cerror_expected_a_before_b(
        clogger* self, tree_location loc, ctoken_kind a, ctoken_kind b)
{
        const cresword_info* b_info = cget_token_kind_info(b);
        cerror(self, CES_ERROR, loc, "expected %s before %s %s",
                cget_token_kind_info(a)->desription, b_info->desription, b_info->kind);
}

extern void cerror_expected_a_or_b_before_c(
        clogger* self, tree_location loc, ctoken_kind a, ctoken_kind b, ctoken_kind c)
{
        const cresword_info* c_info = cget_token_kind_info(c);
        cerror(self, CES_ERROR, loc, "expected %s or %s before %s %s",
                cget_token_kind_info(a)->desription,
                cget_token_kind_info(b)->desription,
                c_info->desription,
                c_info->kind);
}

extern void cerror_expected_one_of(
        clogger* self, tree_location loc, ctoken_kind* v, ssize n, ctoken_kind end)
{
        char buffer[1024];
        sprintf(buffer, "expected one of: ");
        for (ssize i = 0; i < n; i++)
        {
                strcat(buffer, cget_token_kind_info(v[i])->desription);
                if (i + 1 < n)
                        strcat(buffer, ", ");
        }
        const cresword_info* end_info = cget_token_kind_info(end);
        cerror(self, CES_ERROR, loc, "%s before %s %s", 
                buffer, end_info->desription, end_info->kind);
}

extern void cerror_unknown_type_name(clogger* self, const ctoken* id)
{
        cerror(self, CES_ERROR, ctoken_get_loc(id), "unknown type name '%s'",
                tree_get_id_string(self->tree, ctoken_get_string(id)));
}

extern void cerror_expected_type_specifier(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expected type specifier");
}

extern void cerror_invalid_type_specifier(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "invalid combination of type specifiers");
}

extern void cerror_empty_struct(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "empty struct/union is invalid in C99");
}

extern void cerror_empty_enum(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "empty enum is invalid in C99");
}

extern void cerror_expected_expr(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expected an expression");
}

extern void cerror_empty_initializer(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "empty initializer list is invalid in C99");
}

extern void cerror_too_many_initializer_values(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "too many initializer values");
}

extern void cerror_undeclared_identifier(clogger* self, tree_location loc, tree_id name)
{
        cerror(self, CES_ERROR, loc, "undeclared identifier '%s'",
                tree_get_id_string(self->tree, name));
}

extern void cerror_multiple_storage_classes(clogger* self, const cdecl_specs* ds)
{
        cerror(self, CES_ERROR, ds->loc.begin,
                "multiple storage classes in declaration specifiers");
}

extern void cerror_inline_allowed_on_functions_only(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc,
                "'inline' specifier allowed on function declarations only");
}

extern void cerror_invalid_parameter_storage_class(clogger* self, const cdeclarator* d)
{       
        cerror(self, CES_ERROR, d->name_loc, "invalid storage class for parameter '%s'",
                tree_get_id_string(self->tree, d->name));
}

extern void cerror_function_initialized_like_a_variable(clogger* self, const tree_decl* func)
{
        cerror(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function '%s' is initialized like a variable",
                tree_get_id_string(self->tree, tree_get_decl_name(func)));
}

extern void cerror_named_argument_before_ellipsis_required(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "ISO C requires a named argument before '...'");
}

extern void cerror_redefinition(clogger* self, tree_location loc, tree_id id)
{
        cerror(self, CES_ERROR, loc, "redefinition of '%s'", tree_get_id_string(self->tree, id));
}

extern void cerror_decl_redefinition(clogger* self, const tree_decl* decl)
{
        cerror_redefinition(self, tree_get_decl_loc_begin(decl), tree_get_decl_name(decl));
}

extern void cerror_enumerator_value_isnt_constant(
        clogger* self, tree_location loc, tree_id name)
{
        cerror(self, CES_ERROR, loc, "enumerator value for '%s' is not an integer constant",
                tree_get_id_string(self->tree, name));
}

extern void cerror_wrong_king_of_tag(clogger* self, tree_location loc, tree_id name)
{
        cerror(self, CES_ERROR, loc, "'%s' defined as wrong kind of tag",
                tree_get_id_string(self->tree, name));
}

static void cerror_decl(clogger* self, const char* msg, const tree_decl* decl)
{
        cerror(self, CES_ERROR, tree_get_decl_loc_begin(decl), msg,
                tree_get_id_string(self->tree, tree_get_decl_name(decl)));
}

extern void cerror_field_function(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "field '%s' declared as function", field);
}

extern void cerror_invalid_bitfield_type(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "bit-field '%s' has invalid type", field);
}

extern void cerror_bitfield_width_isnt_constant(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "bit-field '%s' width not an integer constant", field);
}

extern void cerror_bitfield_width_is_zero(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "zero width for bit-field '%s'", field);
}

extern void cerror_negative_bitfield_width(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "negative width in bit-field '%s'", field);
}

extern void cerror_bitfield_width_exceeds_type(clogger* self, const tree_decl* field)
{
        cerror_decl(self, "width of '%s' exceeds its type", field);
}

extern void cerror_invalid_storage_class(clogger* self, const tree_decl* decl)
{
        cerror_decl(self, "invalid storage class for '%s'", decl);
}

extern void cerror_different_kind_of_symbol(clogger* self, const tree_decl* decl)
{
        cerror_decl(self, "'%s' redeclared as different kind of symbol", decl);
}

extern void cerror_different_storage_class(clogger* self, const tree_decl* decl)
{
        cerror_decl(self, "redefinition of '%s' with different storage class", decl);
}

extern void cerror_conflicting_types(clogger* self, const tree_decl* decl)
{
        cerror_decl(self, "conflicting types for '%s'", decl);
}

extern void cerror_parameter_name_omitted(clogger* self, const tree_decl* param)
{
        cerror(self, CES_ERROR, tree_get_decl_loc_begin(param), "parameter name omitted");
}

extern void cerror_function_isnt_allowed_here(clogger* self, const tree_decl* func)
{
        cerror(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function definition is not allowed here");
}

extern void cerror_expr_must_have_pointer_to_object_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have pointer-to-object type");
}

extern void cerror_expr_must_have_pointer_to_function_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have pointer-to-function type");
}

extern void cerror_expr_must_have_integral_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have integral type");
}

extern void cerror_expr_must_have_real_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have real type");
}

extern void cerror_expr_must_have_record_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have struct or union type");
}

extern void cerror_expr_must_have_array_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have array type");
}

extern void cerror_expr_must_have_scalar_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have scalar type");
}

extern void cerror_expr_must_have_arithmetic_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have arithmetic type");
}

extern void cerror_expr_must_have_real_or_pointer_to_object_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "expression must have real or pointer-to-object type");
}

extern void cerror_expr_must_be_lvalue_or_function_designator(clogger* self, const tree_expr* e)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(e),
                "expression must be an lvalue or function designator");
}

extern void cerror_expr_must_be_modifiable_lvalue(clogger* self, const tree_expr* e)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(e), "expression must be a modifiable lvalue");
}

extern void cerror_types_are_not_compatible(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "types are not compatible");
}

extern void cerror_subscripted_value_isnt_array(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "subscripted value is not array, pointer, or vector");
}

extern void cerror_incompatible_type_for_argument(clogger* self, tree_location loc, unsigned pos)
{
        cerror(self, CES_ERROR, loc,
                "incompatible type for argument %u in function call", pos);
}

extern void cerror_passing_argument_discards_qualifer(
        clogger* self, tree_location loc, unsigned pos, int quals)
{
        char qstring[128];
        cqet_qual_string(quals, qstring);
        cerror(self, CES_ERROR, loc,
                "passing argument %u discards '%s' qualifier", pos, qstring);
}

extern void cerror_passing_argument_from_incompatible_pointer_type(
        clogger* self, tree_location loc, unsigned pos)
{
        cerror(self, CES_ERROR, loc,
                "passing argument %u from incompatible pointer type", pos);
}

extern void cerror_to_many_arguments(clogger* self, const tree_expr* call)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(call), 
                "too many arguments in function call");
}

extern void cerror_to_few_arguments(clogger* self, const tree_expr* call)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(call),
                "too few arguments in function call");
}

extern void cerror_operand_of_sizeof_is_function(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "operand of sizeof may not be a function");
}

extern void cerror_operand_of_sizeof_is_bitfield(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "operand of sizeof may not be a bitfield");
}

extern void cerror_invalid_binop_operands(clogger* self, tree_location loc, int opcode)
{
        cerror(self, CES_ERROR, loc,
                "invalid operands to binary '%s'", cget_binop_string(opcode));
}

extern void cerror_cmp_of_distinct_pointers(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "comparison of distinct pointer types");
}

extern void cerror_assignment_discards_quals(clogger* self, tree_location loc, int quals)
{
        char qstring[128];
        cqet_qual_string(quals, qstring);
        cerror(self, CES_ERROR, loc, "assignment discards '%s' qualifier", qstring);
}

extern void cerror_assignment_from_incompatible_pointer_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "assignment from incompatible pointer type");
}

extern void cerror_pointer_type_mismatch(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "pointer type mismatch in conditional expression");
}

extern void cerror_type_mismatch(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "type mismatch in conditional expression");
}

extern void cerror_case_stmt_outside_switch(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "a case statement may only be used within a switch");
}

extern void cerror_case_stmt_isnt_constant(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "case label does not reduce to an integer constant");
}

extern void cerror_case_stmt_duplication(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "duplicate case value");
}

extern void cerror_default_stmt_outside_switch(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "a default statement may only be used within a switch");
}

extern void cerror_default_stmt_duplication(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "multiple default labels in one switch");
}

extern void cerror_non_variable_decl_in_for_loop(clogger* self, const tree_decl* decl)
{
        cerror_decl(self, "declaration of non-variable '%s' in 'for' loop", decl);
}

extern void cerror_invalid_storage_class_for_loop_decl(clogger* self, const tree_decl* decl)
{
        tree_decl_storage_class sc = tree_get_decl_storage_class(decl);
        cerror(self, CES_ERROR,
                tree_get_decl_loc_begin(decl),
                "declaration of '%s' variable '%s' in 'for' loop initial declaration",
                cget_decl_storage_class_string(sc),
                tree_get_id_string(self->tree, tree_get_decl_name(decl)));
}

extern void cerror_return_non_void(clogger* self, const tree_expr* expr)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(expr),
                "return with a value, in function returning void");
}

extern void cerror_return_type_doesnt_match(clogger* self, const tree_expr* expr)
{
        cerror(self, CES_ERROR, tree_get_expr_loc(expr),
                "return type does not match the function type");
}

extern void cerror_return_discards_quals(clogger* self, tree_location loc, int quals)
{
        char qstring[128];
        cqet_qual_string(quals, qstring);
        cerror(self, CES_ERROR, loc, "return discards '%s' qualifier", qstring);
}

extern void cerror_return_from_incompatible_pointer_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "return from incompatible pointer type");
}

static void cerror_stmt(clogger* self, const char* msg, const tree_stmt* stmt)
{
        cerror(self, CES_ERROR, tree_get_stmt_loc(stmt).begin, msg);
}

extern void cerror_break_stmt_outside_loop_or_switch(clogger* self, const tree_stmt* stmt)
{
        cerror_stmt(self,
                "a break statement may only be used within a loop or switch", stmt);
}

extern void cerror_continue_stmt_outside_loop(clogger* self, const tree_stmt* stmt)
{
        cerror_stmt(self,
                "a continue statement may only be used within a loop", stmt);
}

extern void cerror_decl_stmt_outside_block(clogger* self, const tree_stmt* stmt)
{
        cerror_stmt(self,
                "a declaration may only be used within a block", stmt);
}

extern void cerror_undefined_label(clogger* self, const tree_decl* label)
{
        cerror_decl(self, "label '%s' used but not defined", label);
}

extern void cerror_incomplete_type(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "incomplete type is not allowed");
}

extern void cerror_array_of_functions(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "array of functions is not allowed");
}

extern void cerror_array_size_isnt_integer(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "size of array has non-integer type");
}

extern void cerror_array_must_be_greater_than_zero(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "size of array must be greater than zero");
}

extern void cerror_function_returning_array(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "function returning array is not allowed");
}

extern void cerror_function_returning_function(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "function returning function is not allowed");
}

extern void cerror_invalid_use_of_restrict(clogger* self, tree_location loc)
{
        cerror(self, CES_ERROR, loc, "invalid use of 'restrict'");
}