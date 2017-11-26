#include "scc/tree/tree-context.h"

extern void tree_init(tree_context* self, tree_target_info* target)
{
        tree_init_ex(self, target, STDALLOC);
}

extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc)
{
        self->_target = target;
        bump_ptr_allocator_init_ex(&self->_alloc, alloc);
        strpool_init_ex(&self->_strings, tree_get_allocator(self));
}

extern void tree_dispose(tree_context* self)
{
        strpool_dispose(&self->_strings);
        bump_ptr_allocator_dispose(&self->_alloc);
}
