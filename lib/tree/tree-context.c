#include "scc/tree/tree-context.h"

extern void tree_init(tree_context* self, tree_target_info* target)
{
        tree_init_ex(self, target, get_std_alloc());
}

extern void tree_init_ex(tree_context* self, tree_target_info* target, allocator* alloc)
{
        self->_target = target;
        bpa_init_ex(&self->_base, alloc);
        strpool_init_ex(&self->_strings, tree_get_allocator(self));
}

extern void tree_dispose(tree_context* self)
{
        strpool_dispose(&self->_strings);
}
