#include "scc/c/c-context.h"
#include "scc/c/c-token.h"
#include "scc/c/c-error.h"
#include "scc/c/c-source.h"
#include <setjmp.h>

extern void c_context_init(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        jmp_buf on_bad_alloc)
{
        c_context_init_ex(self, tree, lookup, on_bad_alloc, STDALLOC);
}

extern void c_context_init_ex(
        c_context* self,
        tree_context* tree,
        file_lookup* lookup,
        jmp_buf on_bad_alloc,
        allocator* alloc)
{
        self->tree = tree;
        mempool_init_ex(&self->memory, NULL, on_bad_alloc, alloc);
        obstack_init_ex(&self->nodes, c_context_get_allocator(self));
        c_source_manager_init(&self->source_manager, lookup, self);
        c_lang_opts_init(&self->lang_opts);
}

extern void c_context_dispose(c_context* self)
{
        c_source_manager_dispose(&self->source_manager);
        obstack_dispose(&self->nodes);
        mempool_dispose(&self->memory);
}