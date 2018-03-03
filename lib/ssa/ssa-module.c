#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-function.h"
#include "scc/core/dseq-instance.h"

extern ssa_module* ssa_new_module(ssa_context* context)
{
        ssa_module* m = ssa_allocate(context, sizeof(ssa_module));
        if (!m)
                return NULL;

        strmap_init_alloc(&m->lookup, ssa_get_alloc(context));
        dseq_init_alloc(&m->globals, ssa_get_alloc(context));
        list_init(&m->defs);
        return m;
}

extern void ssa_module_add_func_def(ssa_module* self, ssa_function* func)
{
        tree_decl* entity = ssa_get_function_entity(func);
        assert(entity);
        strmap_insert(&self->lookup, tree_get_decl_name(entity), func);
        list_push_back(&self->defs, &func->_node);
}

extern void ssa_module_add_global_value(ssa_module* self, ssa_value* global)
{
        assert(ssa_get_value_kind(global) == SVK_STRING);
        dseq_append(&self->globals, global);
}

extern ssa_function* ssa_module_lookup(const ssa_module* self, const tree_decl* func)
{
        strmap_iter res;
        return strmap_find(&self->lookup, tree_get_decl_name(func), &res)
                ? *strmap_iter_value(&res)
                : NULL;
}

extern ssa_value** ssa_get_module_globals_begin(const ssa_module* self)
{
        return (ssa_value**)dseq_begin(&self->globals);
}

extern ssa_value** ssa_get_module_globals_end(const ssa_module* self)
{
        return (ssa_value**)dseq_end(&self->globals);
}