#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-error.h"
#include "scc/c/c-reswords.h"
#include "scc/c/c-info.h"
#include "scc/c/c-macro.h"
#include "scc/c/c-source.h"
#include "scc/c/c-limits.h"
#include "scc/scl/dseq-instance.h"

extern void c_preprocessor_init(
        c_preprocessor* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        self->lexer = NULL;
        c_preprocessor_lexer_stack_init(&self->lexer_stack, context);
        self->lookahead.next_unexpanded_token = NULL;
        self->lookahead.next_expanded_token = NULL;
        strmap_init_alloc(&self->macro_lookup, c_context_get_allocator(context));
        self->reswords = reswords;
        self->source_manager = source_manager;
        self->logger = logger;
        self->context = context;
}

extern void c_preprocessor_dispose(c_preprocessor* self)
{
        c_preprocessor_lexer_stack_dispose(&self->lexer_stack);
        strmap_dispose(&self->macro_lookup);
}

extern serrcode c_preprocessor_enter_source(c_preprocessor* self, c_source* source)
{
        S_ASSERT(source);

        self->lexer = c_preprocessor_lexer_stack_push_token_lexer(
                &self->lexer_stack, self->reswords, self->source_manager, self->logger, self->context);

        return c_token_lexer_enter(&self->lexer->token_lexer, source);
}

static void c_preprocessor_enter_macro(
        c_preprocessor* self, c_macro* macro, tree_location loc)
{
        S_ASSERT(macro);
        macro->used = true;
        self->lexer = c_preprocessor_lexer_stack_push_macro_lexer(
                &self->lexer_stack, self->context, self->reswords, macro, self->logger, loc);
}

extern void c_preprocessor_exit(c_preprocessor* self)
{
        if (self->lexer->kind == CPLK_TOKEN)
                c_source_close(self->lexer->token_lexer.source);
        else if (self->lexer->kind == CPLK_MACRO)
                self->lexer->macro_lexer.macro->used = false;
        else
                S_UNREACHABLE();
        c_preprocessor_lexer_stack_pop_lexer(&self->lexer_stack);
        self->lexer = c_preprocessor_lexer_stack_top(&self->lexer_stack);
}

extern c_macro* c_preprocessor_get_macro(const c_preprocessor* self, tree_id name)
{
        strmap_iter it;
        return strmap_find(&self->macro_lookup, name, &it) ? *strmap_iter_value(&it) : NULL;
}

extern bool c_preprocessor_macro_defined(const c_preprocessor* self, tree_id name)
{
        return c_preprocessor_get_macro(self, name) != NULL;
}

extern bool c_preprocessor_undef(c_preprocessor* self, tree_id name)
{
        return strmap_erase(&self->macro_lookup, name);
}

static c_token* c_preprocess_comment(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_preprocessor_lexer_lex_token(self->lexer);
                if (!t)
                        return NULL;

                if (c_token_is(t, CTK_COMMENT))
                        return c_token_new_wspace(self->context, c_token_get_loc(t), 1);

                if (c_token_is(t, CTK_EOF) 
                        && c_preprocessor_lexer_stack_size(&self->lexer_stack) > 1)
                {
                        c_preprocessor_exit(self);
                        // consume eof of included file
                        continue;
                }

                return t;
        }
}

static c_token* c_preprocess_wspace(c_preprocessor* self, bool skip_eol)
{
        while (1)
        {
                c_token* t = c_preprocess_comment(self);
                if (!t)
                        return NULL;

                if (c_token_is(t, CTK_EOL) && skip_eol)
                        continue;
                if (!c_token_is(t, CTK_WSPACE))
                        return t;
        }
}

static bool c_preprocessor_handle_include(c_preprocessor* self)
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

        ssize size = c_macro_get_tokens_size(macro);
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

static bool c_preprocessor_handle_define(c_preprocessor* self)
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

static bool c_preprocessor_handle_directive(c_preprocessor* self, c_token* tok)
{
        c_token_kind directive = CTK_UNKNOWN;
        if (c_token_is(tok, CTK_ID))
                directive = c_reswords_get_pp_resword_by_ref(self->reswords, c_token_get_string(tok));

        tree_location loc = c_token_get_loc(tok);
        switch (directive)
        {
                case CTK_PP_INCLUDE:
                        return c_preprocessor_handle_include(self);
                case CTK_PP_DEFINE:
                        return c_preprocessor_handle_define(self);
                case CTK_UNKNOWN:
                        c_error_unknown_preprocessor_directive(self->logger, loc);
                        return false;
                default:
                        c_error_unsupported_preprocessor_directive(self->logger, loc);
                        return false;
        }
}

static c_token* c_preprocess_directive(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_preprocess_wspace(self, true);
                if (!t || !c_token_is(t, CTK_HASH))
                        return t;

                // this happens when we get '#' from macro e.g:
                //      #define A #
                //      A
                if (self->lexer->kind != CPLK_TOKEN)
                {
                        c_error_unexpected_hash(self->logger, c_token_get_loc(t));
                        return NULL;
                }

                if (!(t = c_preprocess_wspace(self, false)))
                        return NULL;

                if (c_token_is(t, CTK_EOF))
                        return t;
                else if (c_token_is(t, CTK_EOL))
                        continue;

                self->lexer->token_lexer.in_directive = true;
                bool failed = !c_preprocessor_handle_directive(self, t);
                self->lexer->token_lexer.in_directive = false;
                if (failed)
                        return NULL;
        }
}

typedef struct
{
        ssize num_args;
        ssize num_params;
        c_macro_args* args;
        c_macro* macro;
        tree_location loc;
} c_preprocessing_args;

static void c_preprocessing_args_init(
        c_preprocessing_args* self, c_macro* macro, c_macro_args* args, tree_location loc)
{
        self->num_args = 0;
        self->num_params = c_macro_get_params_size(macro);
        self->args = args;
        self->macro = macro;
        self->loc = loc;
}

static bool c_preprocessor_check_macro_args_overflow(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        if (pp_args->num_args > pp_args->num_params)
        {
                c_error_macro_argument_list_overflow(
                        self->logger, pp_args->macro, pp_args->num_args, pp_args->loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_check_macro_args_underflow(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        if (pp_args->num_args < pp_args->num_params)
        {
                c_error_macro_argument_list_underflow(
                        self->logger, pp_args->macro, pp_args->num_args, pp_args->loc);
                return false;
        }
        return true;
}

static bool c_preprocessor_add_macro_arg(c_preprocessor* self, c_preprocessing_args* pp_args, c_token* arg)
{
        if (!c_preprocessor_check_macro_args_overflow(self, pp_args))
                return false;

        tree_id arg_name = c_macro_get_param(pp_args->macro, pp_args->num_args - 1);
        c_macro_args_add(pp_args->args, arg_name, arg);
        return true;
}

static bool c_preprocessor_append_empty_macro_arg(c_preprocessor* self, c_preprocessing_args* pp_args)
{
        pp_args->num_args++;
        if (!c_preprocessor_check_macro_args_overflow(self, pp_args))
                return false;

        tree_id arg_name = c_macro_get_param(pp_args->macro, pp_args->num_args - 1);
        c_macro_args_set_empty(pp_args->args, arg_name);
        return true;
}

static bool c_preprocessor_read_macro_args(
        c_preprocessor* self, c_macro* macro, c_macro_args* args, tree_location loc)
{
        // true if the argument is empty e.g:
        //      #define A(a)
        //      A()
        bool empty_arg = true;
        int bracket_nesting = 1; // '(' already consumed

        c_preprocessing_args pp_args;
        c_preprocessing_args_init(&pp_args, macro, args, loc);
        while (1)
        {
                c_token* t = c_preprocess_comment(self);
                if (!t)
                        return false;

                if (c_token_is(t, CTK_EOF))
                {
                        c_error_unterminated_macro_argument_list(self->logger, macro, loc);
                        return false;
                }
                else if (c_token_is(t, CTK_LBRACKET))
                        bracket_nesting++;
                else if (c_token_is(t, CTK_RBRACKET))
                {
                        bracket_nesting--;
                        if (bracket_nesting == 0)
                        {
                                if (empty_arg && !c_preprocessor_append_empty_macro_arg(self, &pp_args))
                                        return false;
                                return c_preprocessor_check_macro_args_underflow(self, &pp_args);
                        }
                }
                else if (c_token_is(t, CTK_COMMA) && bracket_nesting == 1)
                {
                        if (empty_arg && !c_preprocessor_append_empty_macro_arg(self, &pp_args))
                                return false;
                        empty_arg = true;
                        continue;
                }

                if (empty_arg)
                        pp_args.num_args++;
                if (!c_preprocessor_add_macro_arg(self, &pp_args, t))
                        return false;
                empty_arg = false;
        }
}

static c_token* c_preprocess_macro(c_preprocessor* self)
{
        while (1)
        {
                c_token* t;
                if ((t = self->lookahead.next_unexpanded_token))
                        self->lookahead.next_unexpanded_token = NULL;
                else if (!(t = c_preprocess_directive(self)))
                        return NULL;

                if (c_token_is(t, CTK_EOM))
                {
                        c_preprocessor_exit(self);
                        continue;
                }
                else if (!c_token_is(t, CTK_ID))
                        return t;

                tree_id id = c_token_get_string(t);
                c_macro* macro = c_preprocessor_get_macro(self, id);
                if (!macro || macro->used)
                        return t;

                // start macro expansion
                tree_location loc = c_token_get_loc(t);
                c_macro_args args;
                c_macro_args_init(&args, self->context);
                if (macro->function_like)
                {
                        c_token* next = c_preprocess_wspace(self, true);
                        if (!next)
                                return NULL;

                        if (!c_token_is(next, CTK_LBRACKET))
                        {
                                self->lookahead.next_unexpanded_token = next;
                                return t;
                        }
                        if (!c_preprocessor_read_macro_args(self, macro, &args, loc))
                        {
                                c_macro_args_dispose(&args);
                                return NULL;
                        }
                }

                c_preprocessor_enter_macro(self, macro, loc);
                bool failed = S_FAILED(c_macro_lexer_substitute_macro_args(
                        &self->lexer->macro_lexer, &args));
                c_macro_args_dispose(&args);
                if (failed)
                        return NULL;
        }
}

static bool c_preprocessor_collect_adjacent_strings(c_preprocessor* self, dseq* result)
{
        while (1)
        {
                c_token* t = c_preprocess_macro(self);
                if (!t)
                        return false;

                if (!c_token_is(t, CTK_CONST_STRING))
                {
                        self->lookahead.next_expanded_token = t;
                        return true;
                }

                dseq_append(result, t);
        }
}

static c_token* c_preprocessor_concat_and_escape_strings(c_preprocessor* self, dseq* strings)
{
        S_ASSERT(dseq_size(strings));

        dseq_u8 concat;
        dseq_u8_init_alloc(&concat, c_context_get_allocator(self->context));
        for (ssize i = 0; i < dseq_size(strings); i++)
        {
                c_token* t = dseq_get(strings, i);
                const char* string = tree_get_id_string(
                        c_context_get_tree_context(self->context), c_token_get_string(t));

                char escaped[C_MAX_LINE_LENGTH + 1];
                ssize size = c_get_escaped_string(escaped, string, strlen(string));
                for (ssize j = 0; j < size - 1; j++)
                        dseq_u8_append(&concat, escaped[j]);
        }
        dseq_u8_append(&concat, '\0');

        tree_id concat_ref = tree_get_id_for_string(
                c_context_get_tree_context(self->context), (char*)dseq_u8_begin(&concat), dseq_u8_size(&concat));
        tree_location loc = c_token_get_loc(dseq_get(strings, 0));
        dseq_u8_dispose(&concat);

        return c_token_new_string(self->context, loc, concat_ref);
}

static c_token* c_preprocess_string(c_preprocessor* self)
{
        c_token* t;
        if ((t = self->lookahead.next_expanded_token))
                self->lookahead.next_expanded_token = NULL;
        else if (!(t = c_preprocess_macro(self)))
                return NULL;

        if (!c_token_is(t, CTK_CONST_STRING))
                return t;

        dseq adjacent_strings;
        dseq_init_alloc(&adjacent_strings, c_context_get_allocator(self->context));
        dseq_append(&adjacent_strings, t);

        c_token* result = c_preprocessor_collect_adjacent_strings(self, &adjacent_strings)
                ? c_preprocessor_concat_and_escape_strings(self, &adjacent_strings)
                : NULL;

        dseq_dispose(&adjacent_strings);
        return result;
}

extern c_token* c_preprocess(c_preprocessor* self)
{
        return c_preprocess_string(self);
}