#include "scc/ssa/ssa-context.h"

extern void ssa_init_context(
        ssa_context* self,
        tree_context* context,
        const tree_target_info* target,
        jmp_buf* on_out_of_mem)
{
        ssa_init_context_ex(self, context, target, on_out_of_mem, get_std_alloc());
}

extern void ssa_init_context_ex(
        ssa_context* self,
        tree_context* context,
        const tree_target_info* target,
        jmp_buf* on_out_of_mem,
        allocator* alloc)
{
        S_ASSERT(on_out_of_mem && target && context);
        self->_target = target;
        self->_tree = context;
        nnull_alloc_init_ex(&self->_alloc, NULL, on_out_of_mem, alloc);
        bpa_init_ex(&self->_base, nnull_alloc_base(&self->_alloc));
}

extern void ssa_dispose_context(ssa_context* self)
{
        bpa_dispose(&self->_base);
}