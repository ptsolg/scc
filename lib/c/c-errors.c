#include "scc/c/c-errors.h"
#include "scc/c/c-error.h"
#include "scc/c/c-limits.h"
#include "scc/c/c-token.h"
#include "scc/c/c-macro.h"
#include "c-misc.h"
#include "scc/c/c-reswords-info.h"
#include "scc/c/c-sema-decl.h"
#include "scc/c/c-context.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-stmt.h"
#include "scc/c/c-sema-conv.h"
#include <ctype.h> // isprint

static const char* c_logger_get_id_string(c_logger* self, tree_id id)
{
        return tree_get_id_string(self->context->tree, id);
}

extern void c_error_cannot_open_source_file(c_logger* self, tree_location loc, const char* file)
{
        c_error(self, CES_ERROR, loc, "cannot open source file '%s'", file);
}

extern void c_error_token_is_too_long(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "token overflowed internal buffer");
}

extern void c_error_missing_closing_quote(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "missing closing quote");
}

extern void c_error_empty_character_constant(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty character constant");
}

extern void c_error_invalid_character_constant(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid character constant");
}

extern void c_error_unclosed_comment(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "comment unclosed at end of file");
}

extern void c_error_stray_symbol(c_logger* self, tree_location loc, int c)
{
        if (isprint(c))
                c_error(self, CES_ERROR, loc, "stray '%c' in program", c);
        else
                c_error(self, CES_ERROR, loc, "stray \\%02X in program", c);
}

extern void c_error_too_deep_include_nesting(c_logger* self)
{
        c_error(self, CES_ERROR, TREE_INVALID_LOC,
                "maximum include nesting is %d", C_MAX_INCLUDE_NESTING);
}

extern void c_error_expected_file_name(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected a file name");
}

extern void c_error_empty_file_name_in_include(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty file name in #include");
}

extern void c_error_unknown_preprocessor_directive(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "unknown preprocessing directive");
}

extern void c_error_unsupported_preprocessor_directive(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "unsupported preprocessor directive");
}

extern void c_error_no_macro_name_given_in_directive(c_logger* self, c_token_kind directive, tree_location loc)
{
        const c_resword_info* info = c_get_token_kind_info(directive);
        c_error(self, CES_ERROR, loc, "no macro name given in %s directive", info->desription);
}

extern void c_error_macro_names_must_be_identifiers(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "macro names must be identifiers");
}

extern void c_error_macro_redefenition(c_logger* self, tree_id name, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'%s' redefenition", c_logger_get_id_string(self, name));
}

extern void c_error_expected_identifier(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected identifier");
}

extern void c_error_missing_closing_bracket_in_macro_parameter_list(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "missing ')' in macro parameter list");
}

extern void c_error_macro_parameters_must_be_comma_separated(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "macro parameters must be comma-separated");
}

extern void c_error_hash2_cannot_appear_at_either_end_of_macro_expansion(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'##' cannot appear at either end of macro expansion");
}

extern void c_error_whitespace_after_macro_name_required(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "ISO C requires whitespaces after the macro name");
}

extern void c_error_invalid_pasting(c_logger* self, c_token* a, c_token* b, tree_location loc)
{
        char a_str[30];
        c_token_to_string(self->context->tree, a, a_str, 30);
        char b_str[30];
        c_token_to_string(self->context->tree, b, b_str, 30);
        c_error(self, CES_ERROR, loc,
                "pasting '%s' and '%s' does not give a valid preprocessing token", a_str, b_str);
}

extern void c_error_unterminated_macro_argument_list(c_logger* self, c_macro* macro, tree_location loc)
{
        const char* macro_name = c_logger_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "unterminated argument list invoking macro '%s'", macro_name);
}

extern void c_error_macro_argument_list_underflow(c_logger* self, c_macro* macro, size_t args_given, tree_location loc)
{
        const char* macro_name = c_logger_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "macro '%s' requires %u arguments, but only %u given",
                macro_name, (uint)c_macro_get_params_size(macro), (uint)args_given);
}

extern void c_error_macro_argument_list_overflow(c_logger* self, c_macro* macro, size_t args_given, tree_location loc)
{
        const char* macro_name = c_logger_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "macro '%s' passed %u arguments, but takes just %u",
                macro_name, (uint)args_given, (uint)c_macro_get_params_size(macro));
}

extern void c_error_hash_is_not_followed_by_a_macro_param(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'#' is not followed by a macro parameter");
}

extern void c_error_ending_directive_without_if(c_logger* self, c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "%s without #if", c_get_token_info(directive)->desription);
}

extern void c_error_extra_tokens_at_end_of_directive(c_logger* self, c_token_kind directive, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "extra tokens at end of %s directive",
                c_get_token_kind_info(directive)->desription);
}

extern void c_error_missing_token_in_expression(c_logger* self, c_token_kind k, tree_location l)
{
        c_error(self, CES_ERROR, l, "missing %s in expression", c_get_token_kind_info(k)->desription);
}

extern void c_error_floating_constant_in_preprocessor_expression(c_logger* self, tree_location l)
{
        c_error(self, CES_ERROR, l, "floating constant in preprocessor expression");
}

extern void c_error_division_by_zero_in_preprocessor_expression(c_logger* self, tree_location l)
{
        c_error(self, CES_ERROR, l, "division by zero in preprocessor expression");
}

extern void c_error_unterminated_directive(c_logger* self, const c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "unterminated %s", c_get_token_info(directive)->desription);
}

extern void c_error_directive_after_else(c_logger* self, const c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "%s after #else", c_get_token_info(directive)->desription);
}

extern void c_error_token_is_not_valid_in_preprocessor_expressions(c_logger* self, const c_token* tok)
{
        c_error(self, CES_ERROR, c_token_get_loc(tok),
                "token %s is not valid in preprocessor expressions", c_get_token_info(tok)->desription);
}

extern void c_error_error_directive(c_logger* self, tree_location loc, const char* msg)
{
        c_error(self, CES_ERROR, loc, "#error %s", msg);
}

extern void c_error_invalid_integer_literal(c_logger* self, tree_location loc, const char* num)
{
        c_error(self, CES_ERROR, loc, "invalid integer literal '%s'", num);
}

extern void c_error_invalid_floating_literal(c_logger* self, tree_location loc, const char* num)
{
        c_error(self, CES_ERROR, loc, "invalid floating literal '%s'", num);
}

extern void c_error_expected_a_before_b(
        c_logger* self, tree_location loc, c_token_kind a, c_token_kind b)
{
        const c_resword_info* b_info = c_get_token_kind_info(b);
        c_error(self, CES_ERROR, loc, "expected %s before %s %s",
                c_get_token_kind_info(a)->desription, b_info->desription, b_info->kind);
}

extern void c_error_expected_a_or_b_before_c(
        c_logger* self, tree_location loc, c_token_kind a, c_token_kind b, c_token_kind c)
{
        const c_resword_info* c_info = c_get_token_kind_info(c);
        c_error(self, CES_ERROR, loc, "expected %s or %s before %s %s",
                c_get_token_kind_info(a)->desription,
                c_get_token_kind_info(b)->desription,
                c_info->desription,
                c_info->kind);
}

extern void c_error_expected_one_of(
        c_logger* self, tree_location loc, c_token_kind* v, size_t n, c_token_kind end)
{
        char buffer[1024];
        sprintf(buffer, "expected one of: ");
        for (size_t i = 0; i < n; i++)
        {
                strcat(buffer, c_get_token_kind_info(v[i])->desription);
                if (i + 1 < n)
                        strcat(buffer, ", ");
        }
        const c_resword_info* end_info = c_get_token_kind_info(end);
        c_error(self, CES_ERROR, loc, "%s before %s %s", 
                buffer, end_info->desription, end_info->kind);
}

extern void c_error_unknown_type_name(c_logger* self, const c_token* id)
{
        c_error(self, CES_ERROR, c_token_get_loc(id), "unknown type name '%s'",
                c_logger_get_id_string(self, c_token_get_string(id)));
}

extern void c_error_expected_type_specifier(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected type specifier");
}

extern void c_error_invalid_type_specifier(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid combination of type specifiers");
}

extern void c_error_empty_struct(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty struct/union is invalid in C99");
}

extern void c_error_empty_enum(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty enum is invalid in C99");
}

extern void c_error_expected_expr(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected an expression");
}

extern void c_error_empty_initializer(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty initializer list is invalid in C99");
}

extern void c_error_too_many_initializer_values(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "too many initializer values");
}

extern void c_error_undeclared_identifier(c_logger* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "undeclared identifier '%s'", c_logger_get_id_string(self, name));
}

extern void c_error_multiple_storage_classes(c_logger* self, const c_decl_specs* ds)
{
        c_error(self, CES_ERROR, ds->loc.begin,
                "multiple storage classes in declaration specifiers");
}

extern void c_error_invalid_specifier(c_logger* self, tree_location loc, c_token_kind spec, int allowed_decl)
{
        const char* decl = "<unknown decl>";
        if (allowed_decl == TDK_VAR)
                decl = "variable";
        else if (allowed_decl == TDK_FUNCTION)
                decl = "function";

        c_error(self, CES_ERROR, loc, "%s specifier allowed on %s declarations only",
                c_get_token_kind_info(spec)->desription, decl);
}

extern void c_error_invalid_parameter_storage_class(c_logger* self, const c_declarator* d)
{       
        c_error(self, CES_ERROR, d->name_loc, "invalid storage class for parameter '%s'",
                c_logger_get_id_string(self, d->name));
}

extern void c_error_variable_declared_thread_local_at_function_scope(c_logger* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "variable '%s' declared '_Thread_local' at function scope",
                c_logger_get_id_string(self, d->name));
}

extern void c_error_variable_declared_register_at_file_scope(c_logger* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "variable '%s' declared 'register' at file scope",
                c_logger_get_id_string(self, d->name));
}

extern void c_error_field_declared_with_storage_specifier(c_logger* self, const c_declarator* d)
{
        c_error(self, CES_ERROR, c_declarator_get_name_loc_or_begin(d),
                "field '%s' declared with storage specifier", c_logger_get_id_string(self, d->name));
}

extern void c_error_function_initialized_like_a_variable(c_logger* self, const tree_decl* func)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function '%s' is initialized like a variable",
                c_logger_get_id_string(self, tree_get_decl_name(func)));
}

extern void c_error_invalid_initializer(c_logger* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(init), "invalid initializer");
}

extern void c_error_initialization_discards_qualifer(c_logger* self, const tree_expr* init, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, tree_get_expr_loc(init),
                "initialization discards '%s' qualifier", qstring);
}

extern void c_error_initialization_from_incompatible_pointer_types(c_logger* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(init),
                "initialization from incompatible pointer types");
}

extern void c_error_braces_around_scalar_initializer(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "braces around scalar initializer");
}

extern void c_error_field_name_not_in_record_initializer(c_logger* self, const tree_designator* d)
{
        c_error(self, CES_ERROR, tree_get_designator_loc(d),
                "field name not in record or union initializer");
}

extern void c_error_array_index_in_non_array_intializer(c_logger* self, const tree_designator* d)
{
        c_error(self, CES_ERROR, tree_get_designator_loc(d), "array index in non-array initializer");
}

extern void c_error_array_index_in_initializer_not_of_integer_type(c_logger* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index),
                "array index in initializer not of integer type");
}

extern void c_error_nonconstant_array_index_in_initializer(c_logger* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index), "nonconstant array index in initializer");
}

extern void c_error_array_index_in_initializer_exceeds_array_bounds(c_logger* self, const tree_expr* index)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(index),
                "array index in initializer exceeds array bounds");
}

extern void c_error_initializer_element_isnt_constant(c_logger* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init), "initializer element is not constant");
}

extern void c_error_initializer_string_is_too_long(c_logger* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init), "initializer-string is too long");
}

extern void c_error_wide_character_array_initialized_from_non_wide_string(c_logger* self, const tree_expr* init)
{
        c_error(self, CES_ERROR, tree_get_expr_loc_begin(init),
                "wide character array initialized from non-wide string");
}

extern void c_error_named_argument_before_ellipsis_required(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "ISO C requires a named argument before '...'");
}

extern void c_error_redefinition(c_logger* self, tree_location loc, tree_id id)
{
        c_error(self, CES_ERROR, loc, "redefinition of '%s'", c_logger_get_id_string(self, id));
}

extern void c_error_decl_redefinition(c_logger* self, const tree_decl* decl)
{
        c_error_redefinition(self, tree_get_decl_loc_begin(decl), tree_get_decl_name(decl));
}

extern void c_error_enumerator_value_isnt_constant(c_logger* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "enumerator value for '%s' is not an integer constant",
                c_logger_get_id_string(self, name));
}

extern void c_error_wrong_king_of_tag(c_logger* self, tree_location loc, tree_id name)
{
        c_error(self, CES_ERROR, loc, "'%s' defined as wrong kind of tag",
                c_logger_get_id_string(self, name));
}

static void c_error_decl(c_logger* self, const char* msg, const tree_decl* decl)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(decl), msg,
                c_logger_get_id_string(self, tree_get_decl_name(decl)));
}

extern void c_error_field_function(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "field '%s' declared as function", field);
}

extern void c_error_invalid_bitfield_type(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "bit-field '%s' has invalid type", field);
}

extern void c_error_bitfield_width_isnt_constant(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "bit-field '%s' width not an integer constant", field);
}

extern void c_error_bitfield_width_is_zero(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "zero width for bit-field '%s'", field);
}

extern void c_error_negative_bitfield_width(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "negative width in bit-field '%s'", field);
}

extern void c_error_bitfield_width_exceeds_type(c_logger* self, const tree_decl* field)
{
        c_error_decl(self, "width of '%s' exceeds its type", field);
}

extern void c_error_invalid_storage_class(c_logger* self, const tree_decl* decl)
{
        c_error_decl(self, "invalid storage class for '%s'", decl);
}

extern void c_error_different_kind_of_symbol(c_logger* self, const tree_decl* decl)
{
        c_error_decl(self, "'%s' redeclared as different kind of symbol", decl);
}

extern void c_error_different_storage_class(c_logger* self, const tree_decl* decl)
{
        c_error_decl(self, "redefinition of '%s' with different storage class", decl);
}

extern void c_error_conflicting_types(c_logger* self, const tree_decl* decl)
{
        c_error_decl(self, "conflicting types for '%s'", decl);
}

extern void c_error_parameter_name_omitted(c_logger* self, const tree_decl* param)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(param), "parameter name omitted");
}

extern void c_error_function_isnt_allowed_here(c_logger* self, const tree_decl* func)
{
        c_error(self, CES_ERROR, tree_get_decl_loc_begin(func),
                "function definition is not allowed here");
}

extern void c_error_expr_must_have_pointer_to_object_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have pointer-to-object type");
}

extern void c_error_expr_must_have_pointer_to_function_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have pointer-to-function type");
}

extern void c_error_expr_must_have_integral_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have integral type");
}

extern void c_error_expr_must_have_real_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have real type");
}

extern void c_error_expr_must_have_record_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have struct or union type");
}

extern void c_error_expr_must_have_array_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have array type");
}

extern void c_error_expr_must_have_scalar_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have scalar type");
}

extern void c_error_expr_must_have_arithmetic_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have arithmetic type");
}

extern void c_error_expr_must_have_real_or_pointer_to_object_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expression must have real or pointer-to-object type");
}

extern void c_error_expr_must_be_lvalue_or_function_designator(c_logger* self, const tree_expr* e)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(e),
                "expression must be an lvalue or function designator");
}

extern void c_error_expr_must_be_modifiable_lvalue(c_logger* self, const tree_expr* e)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(e), "expression must be a modifiable lvalue");
}

extern void c_error_types_are_not_compatible(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "types are not compatible");
}

extern void c_error_subscripted_value_isnt_array(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "subscripted value is not array, pointer, or vector");
}

extern void c_error_incompatible_type_for_argument(c_logger* self, tree_location loc, unsigned pos)
{
        c_error(self, CES_ERROR, loc,
                "incompatible type for argument %u in function call", pos);
}

extern void c_error_passing_argument_discards_qualifer(
        c_logger* self, tree_location loc, unsigned pos, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc,
                "passing argument %u discards '%s' qualifier", pos, qstring);
}

extern void c_error_passing_argument_from_incompatible_pointer_type(
        c_logger* self, tree_location loc, unsigned pos)
{
        c_error(self, CES_ERROR, loc,
                "passing argument %u from incompatible pointer type", pos);
}

extern void c_error_invalid_scalar_initialization(c_logger* self, const tree_expr* e, const c_assignment_conversion_result* r)
{
        switch (r->kind)
        {
                case CACRK_INCOMPATIBLE:
                case CACRK_RHS_NOT_A_RECORD:
                case CACRK_INCOMPATIBLE_RECORDS:
                        c_error_invalid_initializer(self, e);
                        return;
                case CACRK_RHS_NOT_AN_ARITHMETIC:
                        c_error_expr_must_have_arithmetic_type(self, tree_get_expr_loc(e));
                        return;
                case CACRK_QUAL_DISCARTION:
                        c_error_initialization_discards_qualifer(self, e, r->discarded_quals);
                        return;
                case CACRK_INCOMPATIBLE_POINTERS:
                case CACRK_RHS_TRANSACTION_UNSAFE:
                        c_error_initialization_from_incompatible_pointer_types(self, e);
                        return;
                default:
                        UNREACHABLE();
                case CACRK_COMPATIBLE:
                        return;
        }
}

extern void c_error_invalid_argument_assignment(c_logger* self, tree_location loc, unsigned pos, const c_assignment_conversion_result* r)
{
        switch (r->kind)
        {
                case CACRK_RHS_NOT_AN_ARITHMETIC:
                case CACRK_RHS_NOT_A_RECORD:
                case CACRK_INCOMPATIBLE_RECORDS:
                case CACRK_INCOMPATIBLE:
                        c_error_incompatible_type_for_argument(self, loc, pos);
                        return;
                case CACRK_QUAL_DISCARTION:
                        c_error_passing_argument_discards_qualifer(self, loc, pos, r->discarded_quals);
                        return;
                case CACRK_INCOMPATIBLE_POINTERS:
                case CACRK_RHS_TRANSACTION_UNSAFE:
                        c_error_passing_argument_from_incompatible_pointer_type(self, loc, pos);
                        return;
                default:
                        UNREACHABLE();
                case CACRK_COMPATIBLE:
                        return;
        }
}

extern void c_error_invalid_assignment(c_logger* self, tree_location op_loc,
        const tree_expr* rhs, const c_assignment_conversion_result* r)
{
        tree_location rloc = tree_get_expr_loc(rhs);
        switch (r->kind)
        {
                case CACRK_RHS_NOT_AN_ARITHMETIC:
                        c_error_expr_must_have_arithmetic_type(self, rloc);
                        return;
                case CACRK_RHS_NOT_A_RECORD:
                        c_error_expr_must_have_record_type(self, rloc);
                        return;
                case CACRK_INCOMPATIBLE_RECORDS:
                        c_error_types_are_not_compatible(self, rloc);
                        return;
                case CACRK_INCOMPATIBLE:
                        c_error_invalid_binop_operands(self, op_loc, TBK_ASSIGN);
                        return;
                case CACRK_QUAL_DISCARTION:
                        c_error_assignment_discards_quals(self, op_loc, r->discarded_quals);
                        return;
                case CACRK_INCOMPATIBLE_POINTERS:
                case CACRK_RHS_TRANSACTION_UNSAFE:
                        c_error_assignment_from_incompatible_pointer_type(self, op_loc);
                        return;
                default:
                        UNREACHABLE();
                case CACRK_COMPATIBLE:
                        return;
        }
}

extern void c_error_invalid_return_type(c_logger* self, const tree_expr* e, const c_assignment_conversion_result* r)
{
        tree_location loc = tree_get_expr_loc(e);
        switch (r->kind)
        {
                case CACRK_RHS_NOT_AN_ARITHMETIC:
                        c_error_expr_must_have_arithmetic_type(self, loc);
                        return;
                case CACRK_RHS_NOT_A_RECORD:
                        c_error_expr_must_have_record_type(self, loc);
                        return;
                case CACRK_INCOMPATIBLE_RECORDS:
                        c_error_types_are_not_compatible(self, loc);
                        return;
                case CACRK_INCOMPATIBLE:
                        c_error_return_type_doesnt_match(self, e);
                        return;
                case CACRK_QUAL_DISCARTION:
                        c_error_return_discards_quals(self, loc, r->discarded_quals);
                        return;
                case CACRK_INCOMPATIBLE_POINTERS:
                case CACRK_RHS_TRANSACTION_UNSAFE:
                        c_error_return_from_incompatible_pointer_type(self, loc);
                        return;

                default:
                        UNREACHABLE();
                case CACRK_COMPATIBLE:
                        return;
        }
}

extern void c_error_too_many_arguments(c_logger* self, const tree_expr* call)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(call), 
                "too many arguments in function call");
}

extern void c_error_too_few_arguments(c_logger* self, const tree_expr* call)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(call),
                "too few arguments in function call");
}

extern void c_error_operand_of_sizeof_is_function(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "operand of sizeof may not be a function");
}

extern void c_error_operand_of_sizeof_is_bitfield(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "operand of sizeof may not be a bitfield");
}

extern void c_error_invalid_binop_operands(c_logger* self, tree_location loc, int opcode)
{
        c_error(self, CES_ERROR, loc,
                "invalid operands to binary '%s'", c_get_binop_string(opcode));
}

extern void c_error_cmp_of_distinct_pointers(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "comparison of distinct pointer types");
}

extern void c_error_assignment_discards_quals(c_logger* self, tree_location loc, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc, "assignment discards '%s' qualifier", qstring);
}

extern void c_error_assignment_from_incompatible_pointer_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "assignment from incompatible pointer type");
}

extern void c_error_pointer_type_mismatch(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "pointer type mismatch in conditional expression");
}

extern void c_error_type_mismatch(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "type mismatch in conditional expression");
}

extern void c_error_case_stmt_outside_switch(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "a case statement may only be used within a switch");
}

extern void c_error_case_stmt_isnt_constant(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "case label does not reduce to an integer constant");
}

extern void c_error_case_stmt_duplication(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "duplicate case value");
}

extern void c_error_default_stmt_outside_switch(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "a default statement may only be used within a switch");
}

extern void c_error_default_stmt_duplication(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "multiple default labels in one switch");
}

extern void c_error_non_variable_decl_in_for_loop(c_logger* self, const tree_decl* decl)
{
        c_error_decl(self, "declaration of non-variable '%s' in 'for' loop", decl);
}

extern void c_error_invalid_storage_class_for_loop_decl(c_logger* self, const tree_decl* decl)
{
        tree_storage_class sc = tree_get_decl_storage_class(decl);
        c_error(self, CES_ERROR,
                tree_get_decl_loc_begin(decl),
                "declaration of '%s' variable '%s' in 'for' loop initial declaration",
                c_get_decl_storage_class_string(sc),
                c_logger_get_id_string(self, tree_get_decl_name(decl)));
}

extern void c_error_return_non_void(c_logger* self, const tree_expr* expr)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(expr),
                "return with a value, in function returning void");
}

extern void c_error_return_type_doesnt_match(c_logger* self, const tree_expr* expr)
{
        c_error(self, CES_ERROR, tree_get_expr_loc(expr),
                "return type does not match the function type");
}

extern void c_error_return_discards_quals(c_logger* self, tree_location loc, int quals)
{
        char qstring[128];
        c_get_qual_string(quals, qstring);
        c_error(self, CES_ERROR, loc, "return discards '%s' qualifier", qstring);
}

extern void c_error_return_from_incompatible_pointer_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "return from incompatible pointer type");
}

static void c_error_stmt(c_logger* self, const char* msg, const tree_stmt* stmt)
{
        c_error(self, CES_ERROR, tree_get_stmt_loc(stmt).begin, msg);
}

extern void c_error_break_stmt_outside_loop_or_switch(c_logger* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a break statement may only be used within a loop or switch", stmt);
}

extern void c_error_continue_stmt_outside_loop(c_logger* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a continue statement may only be used within a loop", stmt);
}

extern void c_error_decl_stmt_outside_block(c_logger* self, const tree_stmt* stmt)
{
        c_error_stmt(self,
                "a declaration may only be used within a block", stmt);
}

extern void c_error_undefined_label(c_logger* self, const tree_decl* label)
{
        c_error_decl(self, "label '%s' used but not defined", label);
}

extern void c_error_incomplete_type(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "incomplete type is not allowed");
}

extern void c_error_array_of_functions(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "array of functions is not allowed");
}

extern void c_error_array_size_isnt_integer(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "size of array has non-integer type");
}

extern void c_error_array_must_be_greater_than_zero(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "size of array must be greater than zero");
}

extern void c_error_function_returning_array(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "function returning array is not allowed");
}

extern void c_error_function_returning_function(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "function returning function is not allowed");
}

extern void c_error_invalid_use_of_restrict(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid use of 'restrict'");
}

extern void c_error_reffering_volatile_object_is_not_allowed(c_logger* self, tree_location loc, bool in_atomic_block)
{
        const char* context = in_atomic_block ? "_Atomic block" : "_Transaction_safe function";
        c_error(self, CES_ERROR, loc, "volatile objects cannot be used within %s", context);
}

extern void c_error_volatile_param_is_not_allowed(c_logger* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "volatile parameter cannot appear in _Transaction_safe function");
}

extern void c_error_jumping_into_the_atomic_block_is_prohibited(c_logger* self, tree_location loc, c_token_kind stmt_kind)
{
        const char* stmt_name = stmt_kind == CTK_GOTO
                ? "goto"
                : stmt_kind == CTK_CASE ? "case" : "default";

        c_error(self, CES_ERROR, loc,
                "jumping into the body of _Atomic block using %s statement is prohibited", stmt_name);
}