#include "scc/c/c-lexer.h"
#include "scc/c/c-info.h"
#include "scc/c/c-context.h"
#include "scc/c/c-errors.h"
#include "scc/c/c-reswords-info.h"
#include <stdlib.h> // strtoll, strtod, ...
#include <ctype.h> // toupper

extern void c_lexer_init(
        c_lexer* self,
        c_source_manager* source_manager,
        c_logger* logger,
        c_context* context)
{
        c_reswords_init(&self->reswords, context);
        c_preprocessor_init(&self->pp, &self->reswords, source_manager, logger, context);
}

extern errcode c_lexer_enter_source_file(c_lexer* self, c_source* source)
{
        return c_preprocessor_enter_source(&self->pp, source);
}

static_assert(CTK_TOTAL_SIZE == 109, "lexer reswords initialization needs an update");

extern void c_lexer_init_reswords(c_lexer* self)
{
        for (c_token_kind i = CTK_CHAR; i < CTK_CONST_INT; i++)
        {
                const c_resword_info* info = c_get_token_kind_info(i);
                c_reswords_add_resword(&self->reswords, info->string, i);
        }
        for (c_token_kind i = CTK_PP_IF; i < CTK_TOTAL_SIZE; i++)
        {
                const c_resword_info* info = c_get_token_kind_info(i);
                c_reswords_add_pp_resword(&self->reswords, info->string, i);
        }
}

extern void c_lexer_dispose(c_lexer* self)
{
        c_reswords_dispose(&self->reswords);
        c_preprocessor_dispose(&self->pp);
}

static inline const char* c_lexer_get_string(c_lexer* self, tree_id ref)
{
        return tree_get_id_string(c_context_get_tree_context(self->pp.context), ref);
}

static inline bool c_lexer_pp_num_is_floating_constant(c_lexer* self, const c_token* pp_num)
{
        const char* num = c_lexer_get_string(self, c_token_get_string(pp_num));
        bool can_be_hex = false;

        while (*num)
        {
                int c = *num++;

                if (c == '.')
                        return true;

                if (toupper(c) == 'X')
                        can_be_hex = true;

                if (toupper(c) == 'E' && !can_be_hex)
                        return true;
        }
        return false;
}

static bool c_lex_integer_suffix(c_lexer* self, const char* suffix, bool* is_signed, int* ls)
{
        int lc = 0;
        int uc = 0;

        const char* it = suffix;
        while (*it)
        {
                int c = toupper(*it);
                if (c == 'L')
                        lc++;
                else if (c == 'U')
                        uc++;
                else
                        return false;
                it++;
        }

        if (lc > 2 || uc > 1)
                return false;

        *ls = lc;
        *is_signed = uc == 0;
        return true;
}

static inline c_token* c_lex_integer_constant(c_lexer* self, c_token* pp)
{
        const char* num = c_lexer_get_string(self, c_token_get_string(pp));
        int base = num[0] == '0'
                ? toupper(num[1]) == 'X' ? 16 : 8
                : 10;

        char* suffix = NULL;
        uint64_t val = strtoull(num, &suffix, base);

        bool is_signed = true;
        int ls = 0;
        if (!c_lex_integer_suffix(self, suffix, &is_signed, &ls))
        {
                c_error_invalid_integer_literal(self->pp.logger, c_token_get_loc(pp), num);
                return NULL;
        }

        c_token_set_kind(pp, CTK_CONST_INT);
        c_token_set_int(pp, val);
        c_token_set_int_signed(pp, is_signed);
        c_token_set_int_ls(pp, ls);
        return pp;
}

static inline c_token* c_lex_floating_constant(c_lexer* self, c_token* pp)
{
        const char* num = c_lexer_get_string(self, c_token_get_string(pp));
        size_t len = strlen(num);
        char* suffix = NULL;

        float f;
        double d;
        bool is_float = toupper(num[len - 1]) == 'F';
        if (is_float)
                f = strtof(num, &suffix);
        else
                d = strtod(num, &suffix);

        size_t suffix_len = strlen(suffix);
        if ((is_float && suffix_len > 1) || (!is_float && suffix_len))
        {
                c_error_invalid_floating_literal(self->pp.logger, c_token_get_loc(pp), num);
                return NULL;
        }
                
        if (is_float)
        {
                c_token_set_kind(pp, CTK_CONST_FLOAT);
                c_token_set_float(pp, f);
        }
        else
        {
                c_token_set_kind(pp, CTK_CONST_DOUBLE);
                c_token_set_double(pp, d);
        }
        return pp;
}

// converts pp-number to floating or integer constant
static c_token* c_lex_pp_num(c_lexer* self, c_token* num)
{
        return c_lexer_pp_num_is_floating_constant(self, num)
                ? c_lex_floating_constant(self, num)
                : c_lex_integer_constant(self, num);
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
