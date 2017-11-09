#include "scc/tree/tree-context.h"

extern void tree_init_context(tree_context* self)
{
        tree_init_context_ex(self, get_std_alloc());
}

extern void tree_init_context_ex(tree_context* self, allocator* alloc)
{
        bpa_init_ex(&self->_base, alloc);
        strpool_init_ex(tree_get_context_strings(self),
                tree_get_context_allocator(self));
}

extern void tree_dispose_context(tree_context* self)
{
        strpool_dispose(tree_get_context_strings(self));
}
