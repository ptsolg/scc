#include "scc/c/c-context.h"
#include "scc/c/c-token.h"
#include "scc/c/c-error.h"
#include "scc/c/c-source.h"
#include <setjmp.h>

extern void c_context_init(c_context* self, tree_context* tree, jmp_buf on_bad_alloc)
{
        c_context_init_ex(self, tree, on_bad_alloc, STDALLOC);
}

extern void c_context_init_ex(c_context* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc)
{
        self->tree = tree;

        mempool_init_ex(&self->memory, NULL, on_bad_alloc, alloc);
        obstack_init_ex(&self->nodes, c_context_get_allocator(self));
        list_init(&self->sources);
}

extern void c_context_dispose(c_context* self)
{
        while (!list_empty(&self->sources))
                c_delete_source(self, (c_source*)list_pop_back(&self->sources));

        obstack_dispose(&self->nodes);
        mempool_dispose(&self->memory);
}

extern c_source* c_new_source(c_context* context, file_entry* entry)
{
        c_source* s = allocate(c_context_get_allocator(context), sizeof(*s));
        s->begin = TREE_INVALID_LOC;
        s->end = TREE_INVALID_LOC;
        s->file = entry;
        list_node_init(&s->node);
        dseq_u32_init_alloc(&s->lines, c_context_get_allocator(context));
        list_push_back(&context->sources, &s->node);
        return s;
}

extern void c_delete_source(c_context* context, c_source* source)
{
        if (!source)
                return;

        file_close(source->file);
        list_node_remove(&source->node);
        deallocate(c_context_get_allocator(context), source);
}
