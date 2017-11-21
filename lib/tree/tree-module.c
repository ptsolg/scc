#include "scc/tree/tree-module.h"
#include "scc/tree/tree-context.h"

extern tree_module* tree_new_module(tree_context* context)
{
        tree_module* m = tree_allocate(context, sizeof(*m));
        if (!m)
                return NULL;

        tree_init_decl_scope(tree_get_module_globals(m), context, NULL);
        m->_target = tree_get_target(context);
        return m;
}