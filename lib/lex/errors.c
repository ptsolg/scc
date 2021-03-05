#include "errors.h"
#include "scc/c-common/limits.h"
#include "scc/c-common/context.h"
#include "scc/lex/misc.h"
#include "scc/tree/context.h"
#include "scc/lex/reswords-info.h"
#include "macro.h"
#include <ctype.h> // isprint

static const char* c_context_get_id_string(c_context* self, tree_id id)
{
        return tree_get_id_string(self->tree, id);
}

extern void c_error_cannot_open_source_file(c_context* self, tree_location loc, const char* file)
{
        c_error(self, CES_ERROR, loc, "cannot open source file '%s'", file);
}

extern void c_error_token_is_too_long(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "token overflowed internal buffer");
}

extern void c_error_missing_closing_quote(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "missing closing quote");
}

extern void c_error_empty_character_constant(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty character constant");
}

extern void c_error_invalid_character_constant(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "invalid character constant");
}

extern void c_error_unclosed_comment(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "comment unclosed at end of file");
}

extern void c_error_stray_symbol(c_context* self, tree_location loc, int c)
{
        if (isprint(c))
                c_error(self, CES_ERROR, loc, "stray '%c' in program", c);
        else
                c_error(self, CES_ERROR, loc, "stray \\%02X in program", c);
}

extern void c_error_too_deep_include_nesting(c_context* self)
{
        c_error(self, CES_ERROR, TREE_INVALID_LOC,
                "maximum include nesting is %d", C_MAX_INCLUDE_NESTING);
}

extern void c_error_expected_file_name(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected a file name");
}

extern void c_error_empty_file_name_in_include(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty file name in #include");
}

extern void c_error_unknown_preprocessor_directive(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "unknown preprocessing directive");
}

extern void c_error_unsupported_preprocessor_directive(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "unsupported preprocessor directive");
}

extern void c_error_no_macro_name_given_in_directive(c_context* self, c_token_kind directive, tree_location loc)
{
        const c_resword_info* info = c_get_token_kind_info(directive);
        c_error(self, CES_ERROR, loc, "no macro name given in %s directive", info->desription);
}

extern void c_error_macro_names_must_be_identifiers(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "macro names must be identifiers");
}

extern void c_error_macro_redefenition(c_context* self, tree_id name, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'%s' redefenition", c_context_get_id_string(self, name));
}

extern void c_error_expected_identifier(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected identifier");
}

extern void c_error_missing_closing_bracket_in_macro_parameter_list(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "missing ')' in macro parameter list");
}

extern void c_error_macro_parameters_must_be_comma_separated(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "macro parameters must be comma-separated");
}

extern void c_error_hash2_cannot_appear_at_either_end_of_macro_expansion(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'##' cannot appear at either end of macro expansion");
}

extern void c_error_whitespace_after_macro_name_required(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "ISO C requires whitespaces after the macro name");
}

extern void c_error_invalid_pasting(c_context* self, c_token* a, c_token* b, tree_location loc)
{
        char a_str[30];
        c_token_to_string(self->tree, a, a_str, 30);
        char b_str[30];
        c_token_to_string(self->tree, b, b_str, 30);
        c_error(self, CES_ERROR, loc,
                "pasting '%s' and '%s' does not give a valid preprocessing token", a_str, b_str);
}

extern void c_error_unterminated_macro_argument_list(c_context* self, c_macro* macro, tree_location loc)
{
        const char* macro_name = c_context_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "unterminated argument list invoking macro '%s'", macro_name);
}

extern void c_error_macro_argument_list_underflow(c_context* self, c_macro* macro, size_t args_given, tree_location loc)
{
        const char* macro_name = c_context_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "macro '%s' requires %u arguments, but only %u given",
                macro_name, (uint)c_macro_get_params_size(macro), (uint)args_given);
}

extern void c_error_macro_argument_list_overflow(c_context* self, c_macro* macro, size_t args_given, tree_location loc)
{
        const char* macro_name = c_context_get_id_string(self, macro->name);
        c_error(self, CES_ERROR, loc, "macro '%s' passed %u arguments, but takes just %u",
                macro_name, (uint)args_given, (uint)c_macro_get_params_size(macro));
}

extern void c_error_hash_is_not_followed_by_a_macro_param(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "'#' is not followed by a macro parameter");
}

extern void c_error_ending_directive_without_if(c_context* self, c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "%s without #if", c_get_token_info(directive)->desription);
}

extern void c_error_extra_tokens_at_end_of_directive(c_context* self, c_token_kind directive, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "extra tokens at end of %s directive",
                c_get_token_kind_info(directive)->desription);
}

extern void c_error_missing_token_in_expression(c_context* self, c_token_kind k, tree_location l)
{
        c_error(self, CES_ERROR, l, "missing %s in expression", c_get_token_kind_info(k)->desription);
}

extern void c_error_floating_constant_in_preprocessor_expression(c_context* self, tree_location l)
{
        c_error(self, CES_ERROR, l, "floating constant in preprocessor expression");
}

extern void c_error_division_by_zero_in_preprocessor_expression(c_context* self, tree_location l)
{
        c_error(self, CES_ERROR, l, "division by zero in preprocessor expression");
}

extern void c_error_unterminated_directive(c_context* self, const c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "unterminated %s", c_get_token_info(directive)->desription);
}

extern void c_error_directive_after_else(c_context* self, const c_token* directive)
{
        c_error(self, CES_ERROR, c_token_get_loc(directive),
                "%s after #else", c_get_token_info(directive)->desription);
}

extern void c_error_token_is_not_valid_in_preprocessor_expressions(c_context* self, const c_token* tok)
{
        c_error(self, CES_ERROR, c_token_get_loc(tok),
                "token %s is not valid in preprocessor expressions", c_get_token_info(tok)->desription);
}

extern void c_error_error_directive(c_context* self, tree_location loc, const char* msg)
{
        c_error(self, CES_ERROR, loc, "#error %s", msg);
}

extern void c_error_unknown_pragma(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "unknown #pragma directive");
}

extern void c_error_expected_library_name(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected a library name");
}

extern void c_error_empty_library_name(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "empty library name");
}

extern void c_error_invalid_integer_literal(c_context* self, tree_location loc, const char* num)
{
        c_error(self, CES_ERROR, loc, "invalid integer literal '%s'", num);
}

extern void c_error_invalid_floating_literal(c_context* self, tree_location loc, const char* num)
{
        c_error(self, CES_ERROR, loc, "invalid floating literal '%s'", num);
}

extern void c_error_expected_pp_expr(c_context* self, tree_location loc)
{
        c_error(self, CES_ERROR, loc, "expected an expression");
}