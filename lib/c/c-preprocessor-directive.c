#include "scc/c/c-preprocessor-directive.h"
#include "scc/c/c-preprocessor.h"
#include "scc/c/c-context.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-source.h"
#include "scc/c/c-reswords.h"
#include "scc/c/c-macro.h"
#include "scc/c/c-token.h"

extern bool c_preprocessor_handle_directive(c_preprocessor* self, c_token* tok)
{
        c_token_kind directive = CTK_UNKNOWN;
        if (c_token_is(tok, CTK_ID))
                directive = c_reswords_get_pp_resword_by_ref(self->reswords, c_token_get_string(tok));

        tree_location loc = c_token_get_loc(tok);
        switch (directive)
        {
                case CTK_PP_IF:
                        return c_preprocessor_handle_if_directive(self);
                case CTK_PP_IFDEF:
                        return c_preprocessor_handle_ifdef_directive(self);
                case CTK_PP_IFNDEF:
                        return c_preprocessor_handle_ifndef_directive(self);
                case CTK_PP_ELIF:
                        return c_preprocessor_handle_elif_directive(self);
                case CTK_PP_ELSE:
                        return c_preprocessor_handle_else_directive(self);
                case CTK_PP_ENDIF:
                        return c_preprocessor_handle_endif_directive(self);
                case CTK_PP_INCLUDE:
                        return c_preprocessor_handle_include_directive(self);
                case CTK_PP_DEFINE:
                        return c_preprocessor_handle_define_directive(self);
                case CTK_PP_ERROR:
                        return c_preprocessor_handle_error_directive(self);
                case CTK_PP_PRAGMA:
                        return c_preprocessor_handle_pragma_directive(self);
                case CTK_UNKNOWN:
                        c_error_unknown_preprocessor_directive(self->logger, loc);
                        return false;
                default:
                        c_error_unsupported_preprocessor_directive(self->logger, loc);
                        return false;
        }
}

extern bool c_preprocessor_handle_if_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_ifdef_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_ifndef_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_elif_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_else_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_endif_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_include_directive(c_preprocessor* self)
{
        S_ASSERT(self->lexer->kind == CPLK_TOKEN);
        self->lexer->token_lexer.angle_string_expected = true;
        c_token* t = c_preprocess_wspace(self, false);
        self->lexer->token_lexer.angle_string_expected = false;
        if (!t)
                return false;

        tree_location loc = c_token_get_loc(t);
        if (!c_token_is(t, CTK_CONST_STRING) && !c_token_is(t, CTK_ANGLE_STRING))
        {
                c_error_expected_file_name(self->logger, loc);
                return false;
        }

        const char* filename = tree_get_id_string(
                c_context_get_tree_context(self->context), c_token_get_string(t));
        S_ASSERT(filename);

        if (!*filename)
        {
                c_error_empty_file_name_in_include(self->logger, loc);
                return false;
        }

        c_source* source = c_source_find(self->source_manager, filename);
        if (!source)
        {
                c_error_cannot_open_source_file(self->logger, loc, filename);
                return false;
        }

        return S_SUCCEEDED(c_preprocessor_enter_source(self, source));
}

static bool c_preprocessor_read_macro_body(c_preprocessor* self, c_macro* macro)
{
        while (1)
        {
                c_token* t = c_preprocess_wspace(self, false);
                if (!t)
                        return false;

                if (c_token_is(t, CTK_EOL) || c_token_is(t, CTK_EOF))
                        break;

                c_macro_add_token(macro, self->context, t);
        }

        size_t size = c_macro_get_tokens_size(macro);
        if (!size)
                return true;

        c_token* bounds[2];
        bounds[0] = c_macro_get_token(macro, 0);
        bounds[1] = c_macro_get_token(macro, size - 1);
        for (int i = 0; i < 2; i++)
                if (c_token_is(bounds[i], CTK_HASH2))
                {
                        c_error_hash2_cannot_appear_at_either_end_of_macro_expansion(
                                self->logger, c_token_get_loc(bounds[i]));
                        return false;
                }

        return true;
}

static bool c_preprocessor_check_macro_redefinition(const c_preprocessor* self, c_macro* macro)
{
        if (c_preprocessor_macro_defined(self, macro->name))
        {
                c_error_macro_redefenition(self->logger, macro->name, macro->loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_check_macro_name(const c_preprocessor* self, c_token* name)
{
        tree_location loc = c_token_get_loc(name);
        if (c_token_is(name, CTK_EOL) || c_token_is(name, CTK_EOF))
        {
                c_error_no_macro_name_given_in_define(self->logger, loc);
                return false;
        }
        else if (!c_token_is(name, CTK_ID))
        {
                c_error_macro_names_must_be_identifiers(self->logger, loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_read_macro_params(c_preprocessor* self, c_macro* macro)
{
        while (1)
        {
                c_token* t = c_preprocess_wspace(self, false);
                if (!c_token_is(t, CTK_ID))
                {
                        c_error_expected_identifier(self->logger, c_token_get_loc(t));
                        return false;
                }

                c_macro_add_param(macro, self->context, c_token_get_string(t));
                if (!(t = c_preprocess_wspace(self, false)))
                        return false;

                if (c_token_is(t, CTK_RBRACKET))
                        return true;
                else if (c_token_is(t, CTK_EOL) || c_token_is(t, CTK_EOF))
                {
                        c_error_missing_closing_bracket_in_macro_parameter_list(
                                self->logger, c_token_get_loc(t));
                        return false;
                }
                else if (!c_token_is(t, CTK_COMMA))
                {
                        c_error_macro_parameters_must_be_comma_separated(
                                self->logger, c_token_get_loc(t));
                        return false;
                }
        }
}

extern bool c_preprocessor_handle_define_directive(c_preprocessor* self)
{
        c_token* t = c_preprocess_wspace(self, false);
        if (!t)
                return false;

        if (!c_preprocessor_check_macro_name(self, t))
                return false;

        c_macro* macro = c_macro_new(self->context, false, false,
                c_token_get_loc(t), c_token_get_string(t));

        if (!(t = c_preprocess_comment(self)))
                return false;

        bool has_body = true;
        if (c_token_is(t, CTK_LBRACKET))
        {
                macro->function_like = true;
                if (!c_preprocessor_read_macro_params(self, macro))
                        return false;
        }
        else if (c_token_is(t, CTK_EOL) || c_token_is(t, CTK_EOF))
                has_body = false;
        else if (!c_token_is(t, CTK_WSPACE))
        {
                c_error_whitespace_after_macro_name_required(self->logger, c_token_get_loc(t));
                return false;
        }

        if (!c_preprocessor_check_macro_redefinition(self, macro))
                return false;
        if (has_body && !c_preprocessor_read_macro_body(self, macro))
                return false;

        strmap_insert(&self->macro_lookup, macro->name, macro);
        return true;
}

extern bool c_preprocessor_handle_error_directive(c_preprocessor* self)
{
        return false;
}

extern bool c_preprocessor_handle_pragma_directive(c_preprocessor* self)
{
        return false;
}
