#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-context.h"

extern void ssa_init_context(
        ssa_context* self,
        tree_context* context,
        jmp_buf* on_out_of_mem)
{
        ssa_init_context_ex(self, context, on_out_of_mem, get_std_alloc());
}

extern void ssa_init_context_ex(
        ssa_context* self,
        tree_context* context,
        jmp_buf* on_out_of_mem,
        allocator* alloc)
{
        S_ASSERT(on_out_of_mem && target && context);
        self->_target = tree_get_target(context);
        self->_tree = context;
        nnull_alloc_init_ex(&self->_alloc, NULL, on_out_of_mem, alloc);
        bpa_init_ex(&self->_base, nnull_alloc_base(&self->_alloc));
}

extern void ssa_dispose_context(ssa_context* self)
{
        bpa_dispose(&self->_base);
}