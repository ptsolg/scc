#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-function.h"

extern ssa_module* ssa_new_module(ssa_context* context)
{
        ssa_module* m = ssa_allocate(context, sizeof(ssa_module));
        if (!m)
                return NULL;

        htab_init_ex_ptr(&m->_defs, ssa_get_context_alloc(context));
        return m;
}

extern void ssa_module_add_func_def(ssa_module* self, ssa_function* func)
{
        tree_decl* entity = ssa_get_function_entity(func);
        S_ASSERT(entity);
        htab_insert_ptr(&self->_defs, tree_get_decl_name(entity), func);
}

extern ssa_function* ssa_module_find_func_def(ssa_module* self, const tree_decl* func)
{
        hiter res;
        return htab_find(&self->_defs, tree_get_decl_name(func), &res)
                ? hiter_get_ptr(&res)
                : NULL;
}