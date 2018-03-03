#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-context.h"
#include "scc/tree/tree-type.h"

extern void ssa_init(ssa_context* self,
        tree_context* context, jmp_buf on_out_of_mem)
{
        ssa_init_ex(self, context, on_out_of_mem, STDALLOC);
}

extern void ssa_init_ex(ssa_context* self,
        tree_context* context, jmp_buf on_out_of_mem, allocator* alloc)
{
         assert(on_out_of_mem && context);

         self->alloc = alloc;
         self->target = context->target;
         self->tree = context;

         mempool_init_ex(&self->memory, NULL, on_out_of_mem, alloc);
         obstack_init_ex(&self->nodes, ssa_get_alloc(self));
}

extern void ssa_dispose(ssa_context* self)
{
        obstack_dispose(&self->nodes);
        mempool_dispose(&self->memory);
}

extern tree_type* ssa_get_type_for_label(ssa_context* self)
{
        return tree_new_pointer_type(self->tree,
                tree_new_builtin_type(self->tree, TBTK_VOID));
}