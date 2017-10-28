#include "tree-module.h"
#include "tree-context.h"

extern tree_module* tree_new_module(tree_context* context, tree_target_info* target)
{
        tree_module* m = tree_context_fast_allocate(context, sizeof(*m));
        if (!m)
                return NULL;

        tree_init_decl_scope(tree_get_module_globals(m), context, NULL);
        m->_target = target;
        return m;
}