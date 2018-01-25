#include "scc/tree/tree-context.h"
#include "scc/tree/tree-target.h"

extern void tree_init(tree_context* self, tree_target_info* target)
{
        tree_init_ex(self, target, STDALLOC);
}

extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc)
{
        self->_target = target;
        bump_ptr_allocator_init_ex(&self->_alloc, alloc);
        strpool_init_ex(&self->_strings, tree_get_allocator(self));

        for (tree_builtin_type_kind i = TBTK_INVALID; i < TBTK_SIZE; i++)
                tree_init_builtin_type(tree_get_builtin_type(self, i), i);
}

extern void tree_dispose(tree_context* self)
{
        strpool_dispose(&self->_strings);
        bump_ptr_allocator_dispose(&self->_alloc);
}

extern tree_type* tree_get_builtin_type(tree_context* self, tree_builtin_type_kind k)
{
        S_ASSERT(k >= TBTK_INVALID && k < TBTK_SIZE);
        return self->_builtin_types + k;
}

extern tree_type* tree_get_size_type(tree_context* self)
{
        return tree_get_builtin_type(self,
                tree_target_is(self->_target, TTAK_X86_32) ? TBTK_UINT32 : TBTK_UINT64);
}