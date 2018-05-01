#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-context.h"
#include "scc/core/dseq-instance.h"

extern ssa_module* ssa_new_module(ssa_context* context)
{
        ssa_module* m = ssa_allocate(context, sizeof(ssa_module));
        if (!m)
                return NULL;

        dseq_init_alloc(&m->globals, ssa_get_alloc(context));
        dseq_init_alloc(&m->type_decls, ssa_get_alloc(context));
        return m;
}

extern void ssa_add_module_global(ssa_module* self, ssa_value* val)
{
        assert(val);
        dseq_append(&self->globals, val);
}

extern void ssa_add_module_type_decl(ssa_module* self, tree_decl* decl)
{
        assert(decl);
        dseq_append(&self->type_decls, decl);
}

extern ssa_value** ssa_get_module_globals_begin(const ssa_module* self)
{
        return (ssa_value**)dseq_begin(&self->globals);
}

extern ssa_value** ssa_get_module_globals_end(const ssa_module* self)
{
        return (ssa_value**)dseq_end(&self->globals);
}

extern tree_decl** ssa_get_module_type_decls_begin(const ssa_module* self)
{
        return (tree_decl**)dseq_begin(&self->type_decls);
}

extern tree_decl** ssa_get_module_type_decls_end(const ssa_module* self)
{
        return (tree_decl**)dseq_end(&self->type_decls);
}