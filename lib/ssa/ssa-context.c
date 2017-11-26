#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-context.h"

extern void ssa_init(ssa_context* self,
        tree_context* context, jmp_buf on_out_of_mem)
{
        ssa_init_ex(self, context, on_out_of_mem, STDALLOC);
}

extern void ssa_init_ex(ssa_context* self,
        tree_context* context, jmp_buf on_out_of_mem, allocator* alloc)
{
         S_ASSERT(on_out_of_mem && context);

         self->_target = tree_get_target(context);
         self->_tree = context;

         base_allocator_init_ex(&self->base_alloc, NULL, on_out_of_mem, alloc);
         bump_ptr_allocator_init_ex(&self->node_alloc, ssa_get_alloc(self));
}

extern void ssa_dispose(ssa_context* self)
{
        base_allocator_dispose(&self->base_alloc);
}