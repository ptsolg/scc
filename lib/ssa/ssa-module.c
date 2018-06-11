#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-value.h"

extern ssa_module* ssa_new_module(ssa_context* context)
{
        ssa_module* m = ssa_allocate_node(context, sizeof(ssa_module));
        if (!m)
                return NULL;

        ptrvec_init_ex(&m->globals, ssa_get_alloc(context));
        ptrvec_init_ex(&m->type_decls, ssa_get_alloc(context));
        return m;
}

extern void ssa_add_module_global(ssa_module* self, ssa_value* val)
{
        assert(val);
        ptrvec_push(&self->globals, val);
}

extern void ssa_add_module_type_decl(ssa_module* self, tree_decl* decl)
{
        assert(decl);
        ptrvec_push(&self->type_decls, decl);
}

extern void ssa_number_module_values(ssa_module* self)
{
        ssa_id string_uid = 0;
        SSA_FOREACH_MODULE_GLOBAL(self, it, end)
        {
                ssa_value_kind k = ssa_get_value_kind(*it);
                if (k == SVK_FUNCTION)
                        ssa_number_function_values(*it);
                else if (k == SVK_STRING)
                        _ssa_value_base(*it)->id = string_uid++;
        }
}

extern ssa_value** ssa_get_module_globals_begin(const ssa_module* self)
{
        return (ssa_value**)ptrvec_begin(&self->globals);
}

extern ssa_value** ssa_get_module_globals_end(const ssa_module* self)
{
        return (ssa_value**)ptrvec_end(&self->globals);
}

extern tree_decl** ssa_get_module_type_decls_begin(const ssa_module* self)
{
        return (tree_decl**)ptrvec_begin(&self->type_decls);
}

extern tree_decl** ssa_get_module_type_decls_end(const ssa_module* self)
{
        return (tree_decl**)ptrvec_end(&self->type_decls);
}