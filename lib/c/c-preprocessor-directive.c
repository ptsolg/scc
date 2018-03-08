#include "scc/c/c-preprocessor-directive.h"
#include "c-preprocessor-expression.h"
#include "scc/c/c-preprocessor.h"
#include "scc/c/c-context.h"
#include "scc/c/c-limits.h"
#include "scc/c/c-info.h" // c_token_ends_macro
#include "scc/c/c-error.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-source.h"
#include "scc/c/c-reswords.h"
#include "scc/c/c-macro.h"
#include "scc/c/c-token.h"

static bool _c_preprocessor_handle_directive(c_preprocessor* self, c_token* tok)
{
        tree_location loc = c_token_get_loc(tok);
        switch (c_token_get_kind(tok))
        {
                case CTK_PP_IF:
                        return c_preprocessor_handle_if_directive(self, tok);
                case CTK_PP_IFDEF:
                        return c_preprocessor_handle_ifdef_directive(self, tok);
                case CTK_PP_IFNDEF:
                        return c_preprocessor_handle_ifndef_directive(self, tok);
                case CTK_PP_ELIF:
                        return c_preprocessor_handle_elif_directive(self, tok);
                case CTK_PP_ELSE:
                        return c_preprocessor_handle_else_directive(self, tok);
                case CTK_PP_ENDIF:
                        return c_preprocessor_handle_endif_directive(self, tok);
                case CTK_PP_INCLUDE:
                        return c_preprocessor_handle_include_directive(self);
                case CTK_PP_DEFINE:
                        return c_preprocessor_handle_define_directive(self);
                case CTK_PP_ERROR:
                        return c_preprocessor_handle_error_directive(self, tok);
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

extern bool c_preprocessor_handle_directive(c_preprocessor* self, c_token* tok)
{
        size_t current_depth = c_preprocessor_lexer_stack_depth(&self->lexer_stack) - 1;
        assert(self->lexer->kind == CPLK_TOKEN);
        self->lexer->token_lexer.in_directive = true;
        bool result = _c_preprocessor_handle_directive(self, tok);
        c_preprocessor_lexer_stack_get(
                &self->lexer_stack, current_depth)->token_lexer.in_directive = false;
        return result;
}

static bool c_preprocessor_skip_conditional_directive_body(c_preprocessor* self)
{
        c_logger_set_disabled(self->logger);
        int nesting = 1; // assuming we are already in directive' body
        while (1)
        {
                c_token* t = c_preprocessor_lexer_lex_token(self->lexer);
                if (!t)
                        continue;
                if (c_token_is(t, CTK_EOF))
                {
                        c_cond_directive_info* info 
                                = c_preprocessor_lexer_get_conditional_directive(self->lexer);
                        c_logger_set_enabled(self->logger);
                        c_error_unterminated_directive(self->logger, info->token);
                        return false;
                }
                if (!c_token_is(t, CTK_HASH))
                        continue;

                if (!(t = c_preprocess_non_wspace(self)))
                        continue;
                if (!c_token_is(t, CTK_ID))
                        continue;

                c_token_kind directive = c_reswords_get_pp_resword_by_ref(
                        self->reswords, c_token_get_string(t));
                if (directive == CTK_UNKNOWN)
                        continue;

                c_token_set_kind(t, directive);
                switch (directive)
                {
                        case CTK_PP_IFDEF:
                        case CTK_PP_IFNDEF:
                        case CTK_PP_IF:
                                nesting++;
                                break;
                        case CTK_PP_ELIF:
                        case CTK_PP_ELSE:
                        case CTK_PP_ENDIF:
                                nesting--;
                                if (nesting == 0)
                                {
                                        c_logger_set_enabled(self->logger);
                                        return c_preprocessor_handle_directive(self, t);
                                }
                        default:
                                break;

                }
        }
}

static void c_preprocessor_push_conditional_directive(c_preprocessor* self, c_token* token)
{
        c_preprocessor_lexer_push_conditional_directive(self->lexer, token, false);
}

static bool c_preprocessor_finish_conditional_directive(c_preprocessor* self, bool condition)
{
        c_cond_directive_info* info = c_preprocessor_lexer_get_conditional_directive(self->lexer);
        info->condition = condition;
        return condition || c_preprocessor_skip_conditional_directive_body(self);
}

static bool c_preprocessor_require_end_of_directive(c_preprocessor* self, c_token_kind directive)
{
        c_token* t = c_preprocess_non_wspace(self);
        if (!t)
                return false;

        if (!c_token_ends_directive(t))
        {
                c_error_extra_tokens_at_end_of_directive(self->logger, directive, c_token_get_loc(t));
                return false;
        }

        return true;
}

extern bool c_preprocessor_handle_if_directive(c_preprocessor* self, c_token* tok)
{
        c_preprocessor_push_conditional_directive(self, tok);

        c_token* last;
        int_value val;
        if (!(last = c_preprocessor_evaluate_expr(self, &val)))
                return false;
        if (!c_token_ends_directive(last))
        {
                c_error_extra_tokens_at_end_of_directive(self->logger, CTK_PP_IF, c_token_get_loc(last));
                return false;
        }

        return c_preprocessor_finish_conditional_directive(self, !int_is_zero(&val));
}

static c_token* c_preprocessor_read_macro_name(c_preprocessor* self, c_token_kind directive)
{
        c_token* t = c_preprocess_non_wspace(self);
        if (!t)
                return NULL;

        tree_location loc = c_token_get_loc(t);
        if (c_token_ends_directive(t))
        {
                c_error_no_macro_name_given_in_directive(self->logger, directive, c_token_get_loc(t));
                return NULL;
        }
        else if (!c_token_is(t, CTK_ID))
        {
                c_error_macro_names_must_be_identifiers(self->logger, loc);
                return NULL;
        }
        return t;
}

extern bool c_preprocessor_handle_ifdef_directive(c_preprocessor* self, c_token* tok)
{
        c_preprocessor_push_conditional_directive(self, tok);

        c_token* t = c_preprocessor_read_macro_name(self, CTK_PP_IFDEF);
        if (!t || !c_preprocessor_require_end_of_directive(self, CTK_PP_IFDEF))
                return false;

        return c_preprocessor_finish_conditional_directive(self,
                c_preprocessor_macro_defined(self, c_token_get_string(t)));
}

extern bool c_preprocessor_handle_ifndef_directive(c_preprocessor* self, c_token* tok)
{
        c_preprocessor_push_conditional_directive(self, tok);

        c_token* t = c_preprocessor_read_macro_name(self, CTK_PP_IFNDEF);
        if (!t || !c_preprocessor_require_end_of_directive(self, CTK_PP_IFNDEF))
                return false;

        return c_preprocessor_finish_conditional_directive(self,
                !c_preprocessor_macro_defined(self, c_token_get_string(t)));
}

static bool c_preprocessor_check_else_of_endif_directive(c_preprocessor* self, c_token* tok)
{
        if (!c_preprocessor_lexer_get_conditional_directive_stack_depth(self->lexer))
        {
                c_error_ending_directive_without_if(self->logger, tok);
                return false;
        }
        if (c_token_is(tok, CTK_PP_ELIF) || c_token_is(tok, CTK_PP_ELSE))
        {
                c_cond_directive_info* info = c_preprocessor_lexer_get_conditional_directive(self->lexer);
                if (c_token_is(info->token, CTK_PP_ELSE))
                {
                        c_error_directive_after_else(self->logger, tok);
                        return false;
                }
        }
        return true;
}

extern bool c_preprocessor_handle_elif_directive(c_preprocessor* self, c_token* tok)
{
        if (!c_preprocessor_check_else_of_endif_directive(self, tok))
                return false;

        c_cond_directive_info* info = c_preprocessor_lexer_get_conditional_directive(self->lexer);
        info->token = tok;

        c_token* last;
        int_value val;
        if (!(last = c_preprocessor_evaluate_expr(self, &val)))
                return false;
        if (!c_token_ends_directive(last))
        {
                c_error_extra_tokens_at_end_of_directive(self->logger, CTK_PP_ELIF, c_token_get_loc(last));
                return false;
        }

        return c_preprocessor_finish_conditional_directive(self, !info->condition && !int_is_zero(&val));
}

extern bool c_preprocessor_handle_else_directive(c_preprocessor* self, c_token* tok)
{
        if (!c_preprocessor_check_else_of_endif_directive(self, tok))
                return false;

        c_cond_directive_info* info = c_preprocessor_lexer_get_conditional_directive(self->lexer);
        info->token = tok;

        if (!c_preprocessor_require_end_of_directive(self, CTK_PP_ELSE))
                return false;

        return c_preprocessor_finish_conditional_directive(self, !info->condition);
}

extern bool c_preprocessor_handle_endif_directive(c_preprocessor* self, c_token* tok)
{
        if (!c_preprocessor_check_else_of_endif_directive(self, tok))
                return false;
        c_preprocessor_lexer_pop_conditional_directive(self->lexer);
        return c_preprocessor_require_end_of_directive(self, CTK_PP_ENDIF);
}

static c_source* c_preprocessor_find_source(c_preprocessor* self, c_token* tok)
{
        tree_location loc = c_token_get_loc(tok);
        if (!c_token_is(tok, CTK_CONST_STRING) && !c_token_is(tok, CTK_ANGLE_STRING))
        {
                c_error_expected_file_name(self->logger, loc);
                return NULL;
        }

        const char* filename = tree_get_id_string(
                c_context_get_tree_context(self->context), c_token_get_string(tok));
        assert(filename);

        if (!*filename)
        {
                c_error_empty_file_name_in_include(self->logger, loc);
                return NULL;
        }

        c_source* source = c_source_find(self->source_manager, filename);
        if (!source)
        {
                c_error_cannot_open_source_file(self->logger, loc, filename);
                return NULL;
        }
        return source;
}

extern bool c_preprocessor_handle_include_directive(c_preprocessor* self)
{
        assert(self->lexer->kind == CPLK_TOKEN);
        self->lexer->token_lexer.angle_string_expected = true;
        c_token* t = c_preprocess_non_wspace(self);
        self->lexer->token_lexer.angle_string_expected = false;
        if (!t)
                return false;

        c_source* source = c_preprocessor_find_source(self, t);
        if (!source)
                return false;

        if (!c_preprocessor_require_end_of_directive(self, CTK_PP_INCLUDE))
                return false;

        return EC_SUCCEEDED(c_preprocessor_enter_source(self, source));
}

static bool c_preprocessor_read_macro_body(c_preprocessor* self, c_macro* macro)
{
        while (1)
        {
                c_token* t = c_preprocess_non_wspace(self);
                if (!t)
                        return false;

                if (c_token_ends_directive(t))
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

static bool c_preprocessor_read_macro_params(c_preprocessor* self, c_macro* macro)
{
        while (1)
        {
                c_token* t = c_preprocess_non_wspace(self);
                if (!c_token_is(t, CTK_ID))
                {
                        c_error_expected_identifier(self->logger, c_token_get_loc(t));
                        return false;
                }

                c_macro_add_param(macro, self->context, c_token_get_string(t));
                if (!(t = c_preprocess_non_wspace(self)))
                        return false;

                if (c_token_is(t, CTK_RBRACKET))
                        return true;
                else if (c_token_ends_directive(t))
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
        c_token* t = c_preprocessor_read_macro_name(self, CTK_PP_DEFINE);
        if (!t)
                return false;

        c_macro* macro = c_macro_new(self->context, false, false,
                c_token_get_loc(t), c_token_get_string(t));
        if (!(t = c_preprocess_non_comment(self)))
                return false;

        bool has_body = true;
        if (c_token_is(t, CTK_LBRACKET))
        {
                macro->function_like = true;
                if (!c_preprocessor_read_macro_params(self, macro))
                        return false;
        }
        else if (c_token_ends_directive(t))
                has_body = false;
        else if (!c_token_is(t, CTK_WSPACE))
        {
                c_error_whitespace_after_macro_name_required(self->logger, c_token_get_loc(t));
                return false;
        }

        if (has_body && !c_preprocessor_read_macro_body(self, macro))
                return false;

        return c_preprocessor_define_macro(self, macro);
}

extern bool c_preprocessor_handle_error_directive(c_preprocessor* self, c_token* tok)
{
        while (1)
        {
                c_token* t = c_preprocess_non_comment(self);
                if (!t)
                        return false;

                if (c_token_ends_directive(t))
                        break;
        }

        // todo
        c_error_error_directive(self->logger, c_token_get_loc(tok), "");
        return false;
}

extern bool c_preprocessor_handle_pragma_directive(c_preprocessor* self)
{
        return false;
}
