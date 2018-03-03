#include "scc/c/c-macro-lexer.h"
#include "scc/c/c-macro.h"
#include "scc/c/c-limits.h"
#include "scc/c/c-info.h"
#include "scc/c/c-token.h"
#include "scc/c/c-context.h"
#include "scc/c/c-token-lexer.h"
#include "scc/c/c-error.h"
#include "scc/c/c-errors.h"
#include "scc/core/dseq-instance.h"
#include "scc/core/read-write.h"

extern void c_macro_lexer_init(
        c_macro_lexer* self,
        c_context* context,
        const c_reswords* reswords,
        c_macro* macro,
        c_logger* logger,
        tree_location loc)
{
        self->context = context;
        self->reswords = reswords;
        self->macro = macro;
        self->logger = logger;
        self->loc = loc;
        self->pos = NULL;
        dseq_init_alloc(&self->tokens, c_context_get_allocator(context));
}

static c_token* _c_macro_lexer_concat(c_macro_lexer* self, c_token* l, c_token* r, tree_location loc)
{
        c_token_lexer lexer;
        c_token_lexer_init(&lexer, self->reswords, NULL, self->logger, self->context);

        char buf[C_MAX_LINE_LENGTH + 1];
        int n = c_token_to_string(self->context->tree, l, buf, C_MAX_LINE_LENGTH);
        n += c_token_to_string(self->context->tree, r, buf + n, C_MAX_LINE_LENGTH - n);
        buf[n] = '\0';

        sread_cb cb;
        sread_cb_init(&cb, buf);
        readbuf rb;
        readbuf_init(&rb, sread_cb_base(&cb));

        c_token_lexer_enter_char_stream(&lexer, &rb, loc);
        c_logger_set_disabled(self->logger);
        c_token* result = c_token_lexer_lex_token(&lexer);
        c_logger_set_enabled(self->logger);

        if (!result || !c_token_lexer_at_eof(&lexer))
        {
                c_error_invalid_pasting(self->logger, l, r, loc);
                return NULL;
        }

        return result;
}

static c_token* c_macro_lexer_concat(c_macro_lexer* self, const dseq* tokens, tree_location loc)
{
        size_t size = dseq_size(tokens);
        S_ASSERT(size >= 2);

        c_token* concat = _c_macro_lexer_concat(self,
                dseq_get(tokens, 0),
                dseq_get(tokens, 1), loc);
        if (!concat)
                return NULL;

        for (size_t i = 2; i < size; i++)
                if (!(concat = _c_macro_lexer_concat(self, concat, dseq_get(tokens, i), loc)))
                        return NULL;

        return concat;
}

static inline c_token* c_macro_lexer_stringify_macro_arg(
        c_macro_lexer* self, c_macro_args* args, const c_token** arg, tree_location hash_loc)
{
        dseq* tokens;
        if (arg == c_macro_get_tokens_end(self->macro) 
                || !c_token_is(*arg, CTK_ID) 
                || !(tokens = c_macro_args_get(args, c_token_get_string(*arg))))
        {
                c_error_hash_is_not_followed_by_a_macro_param(self->logger, hash_loc);
                return NULL;
        }

        c_token** begin = (c_token**)dseq_begin(tokens);
        c_token** end = (c_token**)dseq_end(tokens);

        c_token** from = begin;
        while (from != end && c_token_is(*from, CTK_WSPACE))
                from++;

        c_token** to = end - 1;
        while (to != begin - 1 && c_token_is(*to, CTK_WSPACE))
                to--;

        char string[C_MAX_LINE_LENGTH + 1];
        *string = '\0';

        int len = 0;
        for (c_token** it = from; it <= to; it++)
                len += c_token_to_string(self->context->tree, *it, string + len, C_MAX_LINE_LENGTH - len);

        tree_id string_id = tree_get_id_for_string(self->context->tree, string, len + 1);
        return c_token_new_string(self->context, self->loc, string_id);
}

extern errcode c_macro_lexer_substitute_macro_args(c_macro_lexer* self, c_macro_args* args)
{
        C_FOREACH_MACRO_TOKEN(self->macro, it, end)
        {
                dseq* tokens;
                c_token* t = *it;
                if (c_token_is(t, CTK_WSPACE) || c_token_is(t, CTK_EOL))
                        continue;
                else if (c_token_is(t, CTK_ID)
                        && (tokens = c_macro_args_get(args, c_token_get_string(t))))
                {
                        for (void** arg_it = dseq_begin(tokens), **arg_end = dseq_end(tokens);
                                arg_it != arg_end; arg_it++)
                        {
                                if (c_token_is(*arg_it, CTK_WSPACE) || c_token_is(*arg_it, CTK_EOL))
                                        continue;
                                dseq_append(&self->tokens,
                                        c_token_copy_with_new_loc(self->context, *arg_it, self->loc));
                        }
                        continue;
                }
                
                if (c_token_is(t, CTK_HASH) && self->macro->function_like)
                        if (!(t = c_macro_lexer_stringify_macro_arg(self, args, ++it, c_token_get_loc(t))))
                                return S_ERROR;

                dseq_append(&self->tokens, t);
        }

        self->pos = (c_token**)dseq_begin(&self->tokens);
        return S_NO_ERROR;
}

extern c_token* c_macro_lexer_lex_token(c_macro_lexer* self)
{
        c_token** end = (c_token**)dseq_end(&self->tokens);
        if (self->pos == end)
                return c_token_new_end_of_macro(self->context, self->loc, self->macro->name);

        if (end - self->pos > 1 && c_token_is(*(self->pos + 1), CTK_HASH2))
        {
                dseq tokens_to_concat;
                dseq_init_alloc(&tokens_to_concat, c_context_get_allocator(self->context));
                dseq_append(&tokens_to_concat, *self->pos);
                self->pos++;

                while (self->pos != end && c_token_is(*self->pos, CTK_HASH2))
                {
                        while (c_token_is(*self->pos, CTK_HASH2))
                                self->pos++;
                        dseq_append(&tokens_to_concat, *self->pos);
                        self->pos++;
                }

                c_token* concat = c_macro_lexer_concat(self, &tokens_to_concat, self->loc);
                dseq_dispose(&tokens_to_concat);
                if (!concat)
                        return false;

                return concat;
        }

        return c_token_copy_with_new_loc(self->context, *self->pos++, self->loc);
}