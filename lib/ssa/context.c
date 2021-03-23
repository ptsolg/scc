#include "scc/ssa/context.h"
#include "scc/core/allocator.h"
#include "scc/tree/context.h"
#include "scc/tree/type.h"

extern void ssa_init(ssa_context* self, tree_context* context, jmp_buf on_out_of_mem)
{
        assert(on_out_of_mem && context);

        self->target = context->target;
        self->tree = context;
        init_stack_alloc(&self->nodes);
}

extern void ssa_dispose(ssa_context* self)
{
        drop_stack_alloc(&self->nodes);
}

extern tree_type* ssa_get_type_for_label(ssa_context* self)
{
        return tree_new_pointer_type(self->tree,
                tree_new_builtin_type(self->tree, TBTK_VOID));
}
