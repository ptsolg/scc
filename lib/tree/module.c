#include "scc/tree/module.h"
#include "scc/tree/context.h"

extern tree_module* tree_new_module(tree_context* context)
{
        tree_module* m = tree_allocate_node(context, sizeof(*m));
        if (!m)
                return NULL;

        tree_init_decl_scope(tree_get_module_globals(m), context, NULL);
        m->target = context->target;
        m->context = context;
        return m;
}

extern tree_decl* tree_module_lookup(const tree_module* self, tree_lookup_kind lk, tree_id id)
{
        return tree_decl_scope_lookup(tree_get_module_cglobals(self), lk, id, false);
}

extern tree_decl* tree_module_lookup_s(const tree_module* self, tree_lookup_kind lk, const char* name)
{
        return tree_module_lookup(self, lk, tree_get_id_for_string(self->context, name));
}