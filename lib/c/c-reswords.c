#include "scc/c/c-reswords.h"
#include "scc/c/c-context.h"
#include "scc/c/c-token-kind.h"

extern void c_reswords_init(c_reswords* self, c_context* context)
{
        strmap_init_alloc(&self->reswords, c_context_get_allocator(context));
        strmap_init_alloc(&self->pp_reswords, c_context_get_allocator(context));
}

extern void c_reswords_dispose(c_reswords* self)
{
        strmap_dispose(&self->reswords);
        strmap_dispose(&self->pp_reswords);
}

extern void c_reswords_add_resword(c_reswords* self, const char* string, int kind)
{
        strref r = STRREF(string);
        strmap_insert(&self->reswords, r, (void*)kind);
}

extern void c_reswords_add_pp_resword(c_reswords* self, const char* string, int kind)
{
        strmap_insert(&self->pp_reswords, STRREF(string), (void*)kind);
}

extern int c_reswords_get_resword(const c_reswords* self, const char* string, size_t len)
{
        return c_reswords_get_resword_by_ref(self, STRREFL(string, len));
}

extern int c_reswords_get_resword_by_ref(const c_reswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->reswords, ref, &res)
                ? (int)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}

extern int c_reswords_get_pp_resword(const c_reswords* self, const char* string, size_t len)
{
        return c_reswords_get_pp_resword_by_ref(self, STRREFL(string, len));
}

extern int c_reswords_get_pp_resword_by_ref(const c_reswords* self, strref ref)
{
        strmap_iter res;
        return strmap_find(&self->pp_reswords, ref, &res)
                ? (c_token_kind)*strmap_iter_value(&res)
                : CTK_UNKNOWN;
}
