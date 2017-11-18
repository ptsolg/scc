#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/tree/tree-module.h"

typedef struct
{
        ssa_value* as_lvalue;
        ssa_value* as_rvalue;
} ssa_var_def;

HTAB_GEN(svd, ssa_var_def);
DSEQ_GEN(htab, htab);

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        self->br_expr_exit = NULL;
        self->function = NULL;

        self->block = NULL;
        ssa_init_builder(&self->builder, self->context, NULL);
        dseq_init_ex_htab(&self->defs, ssa_get_context_alloc(context));
}

extern void ssaizer_dispose(ssaizer* self)
{
        // todo
}

extern void ssaizer_enter_block(ssaizer* self, ssa_block* block)
{
        self->block = block;
        ssa_builder_set_block(&self->builder, block);
}

extern void ssaizer_finish_block(ssaizer* self, ssa_block* block)
{
        ssa_add_function_block(self->function, block);
}

extern void ssaizer_finish_current_block(ssaizer* self)
{
        ssaizer_finish_block(self, self->block);
        self->block = NULL;
}

extern ssa_block* ssaizer_new_block(ssaizer* self)
{
        return ssa_new_block(self->context, ssa_builder_gen_uid(&self->builder), NULL);
}

extern ssa_block* ssaizer_new_br_exit_block(ssaizer* self, tree_type* phi_type)
{
        ssa_block* exit = ssaizer_new_block(self);

        ssa_block* prev = ssa_builder_get_block(&self->builder);
        ssa_builder_set_block(&self->builder, exit);
        ssa_build_phi(&self->builder, phi_type);
        ssa_builder_set_block(&self->builder, prev);

        self->br_expr_exit = exit;
        return exit;
}

static inline htab* ssaizer_get_last_scope(const ssaizer* self)
{
        S_ASSERT(dseq_size(&self->defs));
        return dseq_end_htab(&self->defs) - 1;
}

extern void ssaizer_push_scope(ssaizer* self)
{
        dseq_resize(&self->defs, dseq_size(&self->defs) + 1);
        htab_init_ex_svd(ssaizer_get_last_scope(self),
                ssa_get_context_alloc(self->context));
}

extern void ssaizer_pop_scope(ssaizer* self)
{
        htab_dispose(ssaizer_get_last_scope(self));
        dseq_resize(&self->defs, dseq_size(&self->defs) - 1);
}

extern void ssaizer_set_lvalue_def(ssaizer* self, const tree_decl* var, ssa_value* def)
{
        hiter it;
        htab* last = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        if (htab_find(last, id, &it))
        {
                ((ssa_var_def*)hiter_get_val(&it))->as_lvalue = def;
                return;
        }

        ssa_var_def vd = { .as_lvalue = def, .as_rvalue = NULL };
        htab_insert_svd(last, id, vd);
}

extern ssa_value* ssaizer_get_lvalue_def(ssaizer* self, const tree_decl* var)
{
        hiter it;
        return htab_find(ssaizer_get_last_scope(self), tree_get_decl_name(var), &it)
                ? ((ssa_var_def*)hiter_get_val(&it))->as_lvalue
                : NULL;

}

extern void ssaizer_set_rvalue_def(ssaizer* self, const tree_decl* var, ssa_value* def)
{
        hiter it;
        htab* last = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        if (htab_find(last, id, &it))
        {
                ((ssa_var_def*)hiter_get_val(&it))->as_rvalue = def;
                return;
        }

        ssa_var_def vd = { .as_lvalue = NULL, .as_rvalue = def };
        htab_insert_svd(last, id, vd);
}

extern ssa_value* ssaizer_get_rvalue_def(ssaizer* self, const tree_decl* var)
{
        hiter it;
        return htab_find(ssaizer_get_last_scope(self), tree_get_decl_name(var), &it)
                ? ((ssa_var_def*)hiter_get_val(&it))->as_rvalue
                : NULL;
}

static ssa_function* ssaize_function(ssaizer* self, tree_decl* func)
{
        tree_stmt* body = tree_get_function_body(func);
        if (!body)
                return NULL;

        self->function = ssa_new_function(self->context, func);
        self->block = ssa_new_block(self->context, 0, NULL);
        ssaizer_enter_block(self, self->block);
        ssa_builder_set_uid(&self->builder, 1);

        if (!ssaize_compound_stmt(self, body))
                return NULL;

        return self->function;
}

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module)
{
        ssa_module* m = ssa_new_module(self->context);
        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_DECL_SCOPE_FOREACH(globals, func)
        {
                if (!tree_decl_is(func, TDK_FUNCTION))
                        continue;

                ssa_function* f = ssaize_function(self, func);
                if (!f)
                        continue;

                ssa_module_add_func_def(m, f);
        }
        return m;
}