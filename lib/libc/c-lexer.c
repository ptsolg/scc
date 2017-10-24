#include "c-lexer.h"
#include "c-info.h"
#include "c-tree.h"
#include "c-error.h"
#include "c-reswords.h"
#include <stdlib.h> // strtoll, strtod, ...
#include <ctype.h>  // toupper
#include <libscl/hash.h>

extern void clexer_init(
        clexer*          self,
        csource_manager* source_manager,
        cerror_manager*  error_manager,
        ctree_context*   context)
{
        creswords_init(&self->reswords);
        cpproc_init(&self->pp, &self->reswords, source_manager, error_manager, context);
}

extern serrcode clexer_enter_source_file(clexer* self, csource* source)
{
        return cpproc_enter_source_file(&self->pp, source);
}

S_STATIC_ASSERT(CTK_TOTAL_SIZE == 108, "Update lexer reswords initialization");

static serrcode _clexer_init_reswords(clexer* self, ctoken_kind from, ctoken_kind to)
{
        for (ctoken_kind i = from; i < to; i++)
        {
                const cresword_info* info = cget_token_kind_info(i);
                if (S_FAILED(creswords_add(&self->reswords, info->string, i)))
                        return S_ERROR;
        }
        return S_NO_ERROR;
}

extern serrcode clexer_init_reswords(clexer* self)
{
        for (ctoken_kind i = CTK_CHAR; i < CTK_CONST_INT; i++)
        {
                const cresword_info* info = cget_token_kind_info(i);
                if (S_FAILED(creswords_add(&self->reswords, info->string, i)))
                        return S_ERROR;
        }
        for (ctoken_kind i = CTK_PP_IF; i < CTK_TOTAL_SIZE; i++)
        {
                const cresword_info* info = cget_token_kind_info(i);
                if (S_FAILED(creswords_add_pp(&self->reswords, info->string, i)))
                        return S_ERROR;
        }
        return S_NO_ERROR;
}

extern void clexer_dispose(clexer* self)
{
        creswords_dispose(&self->reswords);
        cpproc_dispose(&self->pp);
}

static inline const char* clexer_get_string(clexer* self, tree_id ref)
{
        return tree_context_get_id(ctree_context_base(self->pp.context), ref);
}

static inline bool clexer_pp_num_is_floating_constant(clexer* self, const ctoken* pp)
{
        const char* num = clexer_get_string(self, ctoken_get_string(pp));
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

static bool clex_integer_suffix(clexer* self, const char* suffix, bool* is_signed, int* ls)
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

static inline ctoken* clex_integer_constant(clexer* self, ctoken* pp)
{
        const char* num = clexer_get_string(self, ctoken_get_string(pp));
        int base = num[0] == '0'
                ? toupper(num[1]) == 'X' ? 16 : 8
                : 10;

        char* suffix = NULL;
        suint64 val  = strtoull(num, &suffix, base);

        bool is_signed = true;
        int ls         = 0;
        if (!clex_integer_suffix(self, suffix, &is_signed, &ls))
        {
                cerror(self->pp.error_manager, CES_ERROR, ctoken_get_loc(pp),
                        "invalid integer literal '%s'", num);
                return NULL;
        }

        ctoken_set_kind(pp, CTK_CONST_INT);
        ctoken_set_int(pp, val);
        ctoken_set_int_signed(pp, is_signed);
        ctoken_set_int_ls(pp, ls);
        return pp;
}

static inline ctoken* clex_floating_constant(clexer* self, ctoken* pp)
{
        const char* num = clexer_get_string(self, ctoken_get_string(pp));
        ssize len       = strlen(num);
        char* suffix    = NULL;

        float f;
        double d;
        bool is_float = toupper(num[len - 1]) == 'F';
        if (is_float)
                f = strtof(num, &suffix);
        else
                d = strtod(num, &suffix);

        ssize suffix_len = strlen(suffix);
        if (is_float && suffix_len > 1 || !is_float && suffix_len)
        {
                cerror(self->pp.error_manager, CES_ERROR, ctoken_get_loc(pp),
                        "invalid floating literal '%s'", num);
                return NULL;
        }
                
        if (is_float)
        {
                ctoken_set_kind(pp, CTK_CONST_FLOAT);
                ctoken_set_float(pp, f);
        }
        else
        {
                ctoken_set_kind(pp, CTK_CONST_DOUBLE);
                ctoken_set_double(pp, d);
        }
        return pp;
}

// converts pp-number to floating or integer constant
static ctoken* clex_pp_num(clexer* self, ctoken* num)
{
        return clexer_pp_num_is_floating_constant(self, num)
                ? clex_floating_constant(self, num)
                : clex_integer_constant(self, num);
}

// converts identifier to keyword
static ctoken* clex_identifier(clexer* self, ctoken* t)
{
        ctoken_kind k = creswords_get_h(&self->reswords, (hval)ctoken_get_string(t));
        if (k != CTK_UNKNOWN)
                ctoken_set_kind(t, k);

        return t;
}

extern ctoken* clex(clexer* self)
{
        ctoken* t = cpreprocess(&self->pp);
        if (!t)
                return NULL;

        ctoken_kind k = ctoken_get_kind(t);
        if (k == CTK_ID)
                return clex_identifier(self, t);
        else if (k == CTK_PP_NUM)
                return clex_pp_num(self, t);

        return t;
}
