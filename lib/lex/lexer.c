#include "scc/lex/lexer.h"
#include "scc/c-common/context.h"
#include "scc/tree/context.h"
#include "scc/lex/misc.h"
#include "scc/lex/reswords-info.h"
#include "errors.h"
#include "numeric-literal.h"

static_assert(CTK_TOTAL_SIZE == 115, "lexer reswords initialization needs an update");

static void c_lexer_add_token(c_lexer* self, c_token_kind k, bool pp)
{
        const c_resword_info* info = c_get_token_kind_info(k);
        if (pp)
                c_reswords_add_pp_resword(&self->reswords, info->string, k);
        else
                c_reswords_add_resword(&self->reswords, info->string, k);
}

static void c_lexer_init_reswords(c_lexer* self)
{
        for (c_token_kind i = CTK_CHAR; i < CTK_CONST_INT; i++)
                c_lexer_add_token(self, i, false);
        for (c_token_kind i = CTK_PP_IF; i <= CTK_PP_PRAGMA; i++)
                c_lexer_add_token(self, i, true);

        if (self->pp.context->lang_opts.ext.tm_enabled)
                for (c_token_kind i = CTK_ATOMIC; i < CTK_TOTAL_SIZE; i++)
                        c_lexer_add_token(self, i, false);
}

extern void c_lexer_init(c_lexer* self, c_context* context)
{
        c_reswords_init(&self->reswords, context);
        c_preprocessor_init(&self->pp, &self->reswords, context);
        c_lexer_init_reswords(self);
}

extern errcode c_lexer_enter_source_file(c_lexer* self, c_source* source)
{
        return c_preprocessor_enter_source(&self->pp, source);
}

extern void c_lexer_dispose(c_lexer* self)
{
        c_reswords_dispose(&self->reswords);
        c_preprocessor_dispose(&self->pp);
}

// converts pp-number to floating or integer constant
static c_token* c_lex_pp_num(c_lexer* self, c_token* num)
{
        const char* num_string = tree_get_id_string(self->pp.context->tree, c_token_get_string(num));
        c_numeric_literal literal;
        c_parse_numeric_literal(num_string, c_token_get_loc(num), &literal, self->pp.context);
        switch (literal.kind)
        {
                case CNLK_SP_FLOATING:
                        c_token_set_kind(num, CTK_CONST_FLOAT);
                        c_token_set_float(num, literal.sp.value);
                        return num;
                case CNLK_DP_FLOATING:
                        c_token_set_kind(num, CTK_CONST_DOUBLE);
                        c_token_set_double(num, literal.dp.value);
                        return num;
                case CNLK_INTEGER:
                        c_token_set_kind(num, CTK_CONST_INT);
                        c_token_set_int(num, literal.integer.value);
                        c_token_set_int_ls(num, literal.integer.num_ls);
                        c_token_set_int_signed(num, literal.integer.is_signed);
                        return num;
                default:
                        return NULL;
        }
}

// converts identifier to keyword
static c_token* c_lex_identifier(c_lexer* self, c_token* t)
{
        c_token_kind k = c_reswords_get_resword_by_ref(&self->reswords, c_token_get_string(t));
        if (k != CTK_UNKNOWN)
                c_token_set_kind(t, k);

        return t;
}

extern c_token* c_lex(c_lexer* self)
{
        c_token* t = c_preprocess(&self->pp);
        if (!t)
                return NULL;

        c_token_kind k = c_token_get_kind(t);
        if (k == CTK_ID)
                return c_lex_identifier(self, t);
        else if (k == CTK_PP_NUM)
                return c_lex_pp_num(self, t);

        return t;
}

extern errcode c_lex_source(c_context* context, file_entry* source, FILE* error, struct vec* result)
{
        errcode code = EC_ERROR;
        c_lexer lexer;
        c_lexer_init(&lexer, context);
        c_source* s = c_source_get_from_file(&context->source_manager, source);
        if (!source || EC_FAILED(c_lexer_enter_source_file(&lexer, s)))
                goto cleanup;

        while (1)
        {
                c_token* t = c_lex(&lexer);
                if (!t)
                        goto cleanup;
                vec_push(result, t);
                if (c_token_is(t, CTK_EOF))
                {
                        code = EC_NO_ERROR;
                        break;
                }
        }
cleanup:
        c_lexer_dispose(&lexer);
        return code;
}
