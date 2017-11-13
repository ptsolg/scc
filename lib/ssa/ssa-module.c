#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/tree/tree-decl.h"

extern ssa_module* ssa_new_module(ssa_context* context)
{
        ssa_module* m = ssa_allocate(context, sizeof(ssa_module));
        if (!m)
                return NULL;

        htab_init_ex_ptr(&m->_defs, ssa_get_context_alloc(context));
        return m;
}

extern void ssa_module_add_def(ssa_module* self, tree_id id, ssa_block* def)
{
        htab_insert_ptr(&self->_defs, id, def);
}

extern void ssa_module_add_func_def(ssa_module* self, const tree_decl* func, ssa_block* def)
{
        ssa_module_add_def(self, tree_get_decl_name(func), def);
}

extern ssa_block* ssa_module_find_def(ssa_module* self, tree_id id)
{
        hiter res;
        return htab_find(&self->_defs, id, &res)
                ? hiter_get_ptr(&res)
                : NULL;
}

extern ssa_block* ssa_module_find_func_def(ssa_module* self, const tree_decl* func)
{
        return ssa_module_find_def(self, tree_get_decl_name(func));
}