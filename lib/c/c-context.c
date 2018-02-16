#include "scc/c/c-context.h"
#include "scc/c/c-token.h"
#include "scc/c/c-error.h"
#include "scc/c/c-source.h"
#include <setjmp.h>

extern void cinit(ccontext* self, tree_context* tree, jmp_buf on_bad_alloc)
{
        cinit_ex(self, tree, on_bad_alloc, STDALLOC);
}

extern void cinit_ex(ccontext* self,
        tree_context* tree, jmp_buf on_bad_alloc, allocator* alloc)
{
        self->tree = tree;

        mempool_init_ex(&self->memory, NULL, on_bad_alloc, alloc);
        obstack_init_ex(&self->nodes, cget_alloc(self));
        list_init(&self->sources);
}

extern void cdispose(ccontext* self)
{
        while (!list_empty(&self->sources))
                csource_delete(self, (csource*)list_pop_back(&self->sources));

        obstack_dispose(&self->nodes);
        mempool_dispose(&self->memory);
}

extern csource* csource_new(ccontext* context, file_entry* entry)
{
        csource* s = allocate(cget_alloc(context), sizeof(*s));
        s->_begin = TREE_INVALID_LOC;
        s->_end = TREE_INVALID_LOC;
        s->_file = entry;
        list_node_init(&s->_node);
        dseq_u32_init_alloc(&s->_lines, cget_alloc(context));
        list_push_back(&context->sources, &s->_node);
        return s;
}

extern void csource_delete(ccontext* context, csource* source)
{
        if (!source)
                return;

        file_close(source->_file);
        list_node_remove(&source->_node);
        deallocate(cget_alloc(context), source);
}
