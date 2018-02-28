#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-error.h"
#include "scc/c/c-info.h"
#include "scc/c/c-macro.h"

static void c_preprocessor_init_builtin_macro(c_preprocessor* self)
{
        // todo: __DATE__ __FILE__ __LINE__ __STDC__ __STDC_HOSTED__ __STDC_MB_MIGHT_NEQ_WC_ 
        // __STDC_VERSION__
        // __TIME__

        // __SCC__ ??
}

extern void c_preprocessor_init(
        c_preprocessor* self,
        const c_reswords* reswords,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        self->reswords = reswords;
        self->state = self->files - 1;
        self->source_manager = source_manager;
        self->logger = logger;
        self->context = context;
        dseq_init_alloc(&self->macro_expansion, c_context_get_allocator(context));
        strmap_init_alloc(&self->macro_lookup, c_context_get_allocator(context));
        c_preprocessor_init_builtin_macro(self);
}

extern void c_preprocessor_dispose(c_preprocessor* self)
{
        while (self->state != self->files - 1)
        {
                c_source_close(self->state->source);
                self->state--;
        }
}

extern serrcode c_preprocessor_enter(c_preprocessor* self, c_source* source)
{
        if (!source)
                return S_ERROR;

        if (self->state - self->files >= C_MAX_INCLUDE_NESTING)
        {
                c_error_too_deep_include_nesting(self->logger);
                return S_ERROR;
        }

        c_preprocessor_state* next = self->state + 1;
        c_preprocessor_lexer* pp_lexer = &next->lexer;
        c_preprocessor_lexer_init(
                pp_lexer, self->reswords, self->source_manager, self->logger, self->context);

        if (S_FAILED(c_preprocessor_lexer_enter(pp_lexer, source)))
                return S_ERROR;

        self->state = next;
        self->state->source = source;
        self->state->hash_expected = true;
        self->state->in_directive = false;
        return S_NO_ERROR;
}

extern void c_preprocessor_exit(c_preprocessor* self)
{
        S_ASSERT(self->state != self->files);
        c_source_close(self->state->source);
        self->state--;
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

extern void c_preprocessor_undef(const c_preprocessor* self, tree_id name)
{
        S_UNREACHABLE();
}

static c_token* c_preprocess_comment(c_preprocessor* self)
{
        while (1)
        {
                c_token* t = c_preprocessor_lexer_lex_token(&self->state->lexer);
                if (!t)
                        return NULL;

                if (c_token_is(t, CTK_COMMENT))
                        return c_token_new_wspace(self->context, c_token_get_loc(t), 1);
               
                if (c_token_is(t, CTK_EOF) && self->state != self->files)
                {
                        c_preprocessor_exit(self);
                        // consume eof of included file
                        continue;
                }
                else if (c_token_is(t, CTK_EOL))
                        self->state->hash_expected = true;
                else if (!c_token_is(t, CTK_WSPACE))
                {
                        if (c_token_is(t, CTK_HASH) 
                                && !self->state->hash_expected
                                && !self->state->in_directive)
                        {
                                c_error_unexpected_hash(self->logger, c_token_get_loc(t));
                                return NULL;
                        }
                        self->state->hash_expected = false;
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
        self->state->lexer.angle_string_expected = true;
        c_token* t = c_preprocess_wspace(self, false);
        self->state->lexer.angle_string_expected = false;
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

        return S_SUCCEEDED(c_preprocessor_enter(self, source));
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
                S_UNREACHABLE();
                // todo: function-like macro
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
                directive = c_reswords_get_pp_by_ref(self->reswords, c_token_get_string(tok));

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

                if (!(t = c_preprocess_wspace(self, false)))
                        return NULL;

                if (c_token_is(t, CTK_EOF))
                        return t;
                else if (c_token_is(t, CTK_EOL))
                        continue;

                self->state->in_directive = true;
                bool failed = !c_preprocessor_handle_directive(self, t);
                self->state->in_directive = false;
                if (failed)
                        return NULL;
        }
}

static c_token* c_preprocessor_pop_macro_expansion(c_preprocessor* self)
{
        c_token* last = *(dseq_end(&self->macro_expansion) - 1);
        dseq_resize(&self->macro_expansion, dseq_size(&self->macro_expansion) - 1);
        return last;
}

static inline void c_preprocessor_push_macro_expansion(c_preprocessor* self, c_token* t)
{
        dseq_append(&self->macro_expansion, t);
}

static c_token* _c_preprocessor_concat(c_preprocessor* self, c_token* l, c_token* r, tree_location loc)
{
        c_preprocessor_lexer lexer;
        c_preprocessor_lexer_init(&lexer, self->reswords, NULL, self->logger, self->context);

        char buf[C_MAX_LINE_LENGTH + 1];
        int n = c_token_to_string(self->context->tree, l, buf, C_MAX_LINE_LENGTH);
        n += c_token_to_string(self->context->tree, r, buf + n, C_MAX_LINE_LENGTH - n);
        buf[n] = '\0';

        sread_cb cb;
        sread_cb_init(&cb, buf);
        readbuf rb;
        readbuf_init(&rb, sread_cb_base(&cb));

        c_preprocessor_lexer_enter_char_stream(&lexer, &rb, loc);
        c_logger_set_disabled(self->logger);
        c_token* result = c_preprocessor_lexer_lex_token(&lexer);
        c_logger_set_enabled(self->logger);
       
        if (!result || !c_preprocessor_lexer_at_eof(&lexer))
        {
                c_error_invalid_pasting(self->logger, l, r, loc);
                return NULL;
        }

        return result;
}

static c_token* c_preprocessor_concat(c_preprocessor* self, const dseq* tokens, tree_location loc)
{
        ssize size = dseq_size(tokens);
        S_ASSERT(size >= 2);

        c_token* concat = _c_preprocessor_concat(self,
                dseq_get(tokens, size - 1),
                dseq_get(tokens, size - 2), loc);
        if (!concat)
                return NULL;

        for (ssize i = size - 3; i != -1; i--)
                if (!(concat = _c_preprocessor_concat(self, concat, dseq_get(tokens, i), loc)))
                        return NULL;

        return concat;
}

static bool _c_preprocessor_expand_macro(c_preprocessor* self,
        c_macro* macro, tree_location loc, strmap* used_macro)
{
        if (!c_macro_get_tokens_size(macro))
                return true;

        strmap_insert(used_macro, macro->name, macro);
        c_token** it = c_macro_get_tokens_end(macro) - 1;
        c_token** end = c_macro_get_tokens_begin(macro) - 1;
        while (it != end)
        {
                // handle a ## ... ## b ## c ...
                if (it - end > 1 && c_token_is(*(it - 1), CTK_HASH2))
                {
                        dseq tokens_to_concat;
                        dseq_init_alloc(&tokens_to_concat, c_context_get_allocator(self->context));
                        dseq_append(&tokens_to_concat, *it);
                        it--;

                        while (it != end && c_token_is(*it, CTK_HASH2))
                        {
                                while (c_token_is(*it, CTK_HASH2))
                                        it--;
                                dseq_append(&tokens_to_concat, *it);
                                it--;
                        }

                        c_token* concat = c_preprocessor_concat(self, &tokens_to_concat, loc);
                        dseq_dispose(&tokens_to_concat);
                        if (!concat)
                                return false;

                        c_preprocessor_push_macro_expansion(self, concat);
                        continue;
                }

                c_token* t = *it--;
                if (c_token_is(t, CTK_ID))
                {
                        c_macro* m = c_preprocessor_get_macro(self, c_token_get_string(t));
                        strmap_iter placeholder;
                        if (m && !strmap_find(used_macro, m->name, &placeholder))
                        {
                                _c_preprocessor_expand_macro(self, m, loc, used_macro);
                                continue;
                        }
                }
                c_token* copy = c_token_copy(self->context, t);
                c_token_set_loc(copy, loc);
                c_preprocessor_push_macro_expansion(self, copy);
        }

        strmap_erase(used_macro, macro->name);
        return true;
}

static bool c_preprocessor_expand_macro(c_preprocessor* self, c_macro* macro, tree_location loc)
{
        S_ASSERT(dseq_size(&self->macro_expansion) == 0);
        strmap used_macro;
        strmap_init_alloc(&used_macro, c_context_get_allocator(self->context));
        bool result = _c_preprocessor_expand_macro(self, macro, loc, &used_macro);
        strmap_dispose(&used_macro);
        return result;
}

static c_token* c_preprocess_macro(c_preprocessor* self)
{
        while (1)
        {
                if (dseq_size(&self->macro_expansion))
                        return c_preprocessor_pop_macro_expansion(self);

                c_token* t = c_preprocess_directive(self);
                if (!t || !c_token_is(t, CTK_ID))
                        return t;

                c_macro* macro;
                tree_id name = c_token_get_string(t);
                if (!(macro = c_preprocessor_get_macro(self, name)))
                        return t;
                if (!c_preprocessor_expand_macro(self, macro, c_token_get_loc(t)))
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
                        c_preprocessor_push_macro_expansion(self, t);
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
        c_token* t = c_preprocess_macro(self);
        if (!t)
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