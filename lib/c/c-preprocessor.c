#include "scc/c/c-preprocessor.h"
#include "scc/c/c-token.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-context.h"
#include "scc/c/c-info.h"

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
        dseq_init_alloc(&self->expansion, c_context_get_allocator(context));
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
        c_preprocessor_lexer* pplexer = &next->lexer;
        c_preprocessor_lexer_init(
                pplexer,
                self->reswords,
                self->source_manager,
                self->logger, 
                self->context);
        if (S_FAILED(c_preprocessor_lexer_enter(pplexer, source)))
                return S_ERROR;

        self->state = next;
        self->state->source = source;
        self->state->nhash = 0;
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

static c_token* c_preprocess_wspace(c_preprocessor* self, bool skip_eol)
{
        while (1)
        {
                c_token* t = c_preprocessor_lexer_lex_token(&self->state->lexer);
                if (!t)
                        return NULL;

                c_token_kind k = c_token_get_kind(t);
                if (k == CTK_WSPACE || k == CTK_COMMENT || (k == CTK_EOL && skip_eol))
                        continue;
                else if (k == CTK_EOF && self->state != self->files)
                {
                        // consume eof of included source file
                        c_preprocessor_exit(self);
                        continue;
                }

                return t;
        }
}

static c_token* c_preprocess_directive(c_preprocessor*);

static c_token* c_preprocess_include(c_preprocessor* self)
{
        self->state->lexer.angle_string_expected = true;
        c_token* t = c_preprocess_wspace(self, false);
        self->state->lexer.angle_string_expected = false;
        if (!t)
                return NULL;

        tree_location token_loc = c_token_get_loc(t);
        if (!c_token_is(t, CTK_CONST_STRING) && !c_token_is(t, CTK_ANGLE_STRING))
        {
                c_error_expected_file_name(self->logger, token_loc);
                return NULL;
        }

        tree_id ref = c_token_get_string(t);
        const char* filename = tree_get_id_string(c_context_get_tree_context(self->context), ref);
        S_ASSERT(filename);

        if (!*filename)
        {
                c_error_empty_file_name_in_include(self->logger, token_loc);
                return NULL;
        }

        c_source* source = c_source_find(self->source_manager, filename);
        if (!source)
        {
                c_error_cannot_open_source_file(self->logger, token_loc, filename);
                return NULL;
        }

        if (S_FAILED(c_preprocessor_enter(self, source)))
                return NULL;

        return c_preprocess_directive(self);
}

static c_token* c_preprocess_directive(c_preprocessor* self)
{
        c_token* t = c_preprocess_wspace(self, true);
        if (!t || !c_token_is(t, CTK_HASH))
                return t;

        if (!self->state->hash_expected)
        {
                c_error_unexpected_hash(self->logger, c_token_get_loc(t));
                return NULL;
        }

        if (!(t = c_preprocess_wspace(self, false)))
                return NULL;

        c_token_kind directive = CTK_UNKNOWN;
        if (c_token_is(t, CTK_ID))
                directive = c_reswords_get_pp_by_ref(self->reswords, c_token_get_string(t));

        if (directive == CTK_UNKNOWN)
        {
                c_error_unknown_preprocessor_directive(self->logger, c_token_get_loc(t));
                return false;
        }

        self->state->in_directive = true;
        if (directive == CTK_PP_INCLUDE)
                t = c_preprocess_include(self);
        else
        {
                t = NULL;
                c_error_unsupported_preprocessor_directive(self->logger, c_token_get_loc(t));
        }
        self->state->in_directive = false;
        return t;
}

static c_token* c_preprocess_macro_id(c_preprocessor* self)
{
        if (dseq_size(&self->expansion))
        {
                c_token* last = *(dseq_end(&self->expansion) - 1);
                dseq_resize(&self->expansion, dseq_size(&self->expansion) - 1);
                return last;
        }

        c_token* t = c_preprocess_directive(self);
        if (!t)
                return NULL;

        if (!c_token_is(t, CTK_ID))
                return t;

        if (0) // token is macro id
        {
                // push back replacement
                // ...
                // return (ctoken*)list_pop_front(&self->expansion);
        }
        return t;
}

static inline void c_preprocessor_unget_macro_id(c_preprocessor* self, c_token* t)
{
        dseq_append(&self->expansion, t);
}

static bool c_preprocessor_collect_adjacent_strings(c_preprocessor* self, dseq* result)
{
        while (1)
        {
                c_token* t = c_preprocess_macro_id(self);
                if (!t)
                        return false;

                if (!c_token_is(t, CTK_CONST_STRING))
                {
                        c_preprocessor_unget_macro_id(self, t);
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
        c_token* t = c_preprocess_macro_id(self);
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