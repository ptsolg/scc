#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/tree/tree-decl.h"
#include "scc/tree/tree-module.h"

DSEQ_GEN(sbi, ssa_block_info);

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        dseq_init_ex_sbi(&self->block_info, ssa_get_context_alloc(context));
}

extern void ssaizer_dispose(ssaizer* self)
{
        // todo
}

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block)
{
        dseq_resize(&self->block_info, dseq_size(&self->block_info) + 1);

        ssa_block_info* last = dseq_end_sbi(&self->block_info) - 1;
        last->block = block;
        htab_init_ex_ptr(&last->defs, ssa_get_context_alloc(self->context));

        self->block = block;
        self->defs = &last->defs;
}

extern void ssaizer_exit_block(ssaizer* self)
{
        ssize size = dseq_size(&self->block_info);
        S_ASSERT(size);
        dseq_resize(&self->block_info, size - 1);

        ssa_block_info* last = dseq_begin_sbi(&self->block_info) + (size - 1);
        self->block = last->block;
        self->defs = &last->defs;
}

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* val)
{
        htab_insert_ptr(self->defs, tree_get_decl_name(var), val);
}

extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var)
{
        sptrdiff n = (sptrdiff)dseq_size(&self->block_info);
        if (!n)
                return NULL;

        tree_id id = tree_get_decl_name(var);
        n--;
        for (; n > -1; n--)
        {
                ssa_block_info* info = dseq_begin_sbi(&self->block_info) + n;
                hiter res;
                if (htab_find(&info->defs, id, &res))
                        return hiter_get_ptr(&res);
        }
        return NULL;
}

extern ssa_block* ssaize_function(ssaizer* self, const tree_decl* func)
{
        S_ASSERT(func && tree_decl_is(func, TDK_FUNCTION));
        tree_stmt* body = tree_get_function_body(func);
        if (!body)
                return NULL;

        S_ASSERT(dseq_size(&self->block_info) == 0);
        return ssaize_compound_stmt(self, body);
}

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module)
{
        ssa_module* m = ssa_new_module(self->context);
        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_DECL_SCOPE_FOREACH(globals, func)
        {
                if (!tree_decl_is(func, TDK_FUNCTION))
                        continue;

                ssa_block* b = ssaize_function(self, func);
                if (!b)
                        ; // todo
                if (S_FAILED(ssa_module_add_func_def(m, func, b)))
                        ; // todo
        }

        return m;
}