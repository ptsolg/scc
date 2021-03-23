#include "scc/tree/context.h"
#include "scc/core/allocator.h"
#include "scc/core/strpool.h"
#include "scc/tree/target.h"

extern void tree_init(tree_context* self, tree_target_info* target)
{
        self->target = target;
        init_stack_alloc(&self->nodes);
        init_strpool(&self->strings);
        for (tree_builtin_type_kind i = TBTK_INVALID; i < TBTK_SIZE; i++)
                tree_init_builtin_type(tree_get_builtin_type(self, i), i);
}

extern void tree_dispose(tree_context* self)
{
        drop_strpool(&self->strings);
        drop_stack_alloc(&self->nodes);
}

extern tree_type* tree_get_builtin_type(tree_context* self, tree_builtin_type_kind k)
{
        assert(k >= TBTK_INVALID && k < TBTK_SIZE);
        return self->builtin_types + k;
}

extern tree_type* tree_get_size_type(tree_context* self)
{
        return tree_get_builtin_type(self,
                tree_target_is(self->target, TTAK_X86_32) ? TBTK_UINT32 : TBTK_UINT64);
}

extern tree_type* tree_get_ptrdiff_type(tree_context* self)
{
        return tree_get_builtin_type(self,
                tree_target_is(self->target, TTAK_X86_32) ? TBTK_INT32 : TBTK_INT64);
}
