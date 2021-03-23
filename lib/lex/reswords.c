#include "scc/lex/reswords.h"
#include "scc/c-common/context.h"
#include "scc/core/hash.h"
#include "scc/lex/token-kind.h"

extern void c_reswords_init(c_reswords* self, c_context* context)
{
        hashmap_init(&self->reswords);
        hashmap_init(&self->pp_reswords);
}

extern void c_reswords_dispose(c_reswords* self)
{
        hashmap_drop(&self->reswords);
        hashmap_drop(&self->pp_reswords);
}

extern void c_reswords_add_resword(c_reswords* self, const char* string, int kind)
{
        hashmap_insert(&self->reswords, strhash(string), (void*)(size_t)kind);
}

extern void c_reswords_add_pp_resword(c_reswords* self, const char* string, int kind)
{
        hashmap_insert(&self->pp_reswords, strhash(string), (void *)(size_t)kind);
}

extern int c_reswords_get_resword(const c_reswords* self, const char* string, size_t len)
{
        return c_reswords_get_resword_by_ref(self, hash(string, len));
}

extern int c_reswords_get_resword_by_ref(const c_reswords* self, unsigned ref)
{
        struct hashmap_entry* entry = hashmap_lookup(&self->reswords, ref);
        return entry ? (int)(size_t)entry->value : CTK_UNKNOWN;
}

extern int c_reswords_get_pp_resword(const c_reswords* self, const char* string, size_t len)
{
        return c_reswords_get_pp_resword_by_ref(self, hash(string, len));
}

extern int c_reswords_get_pp_resword_by_ref(const c_reswords* self, unsigned ref)
{
        struct hashmap_entry* entry = hashmap_lookup(&self->pp_reswords, ref);
        return entry ? (c_token_kind)(size_t)entry->value : CTK_UNKNOWN;
}
