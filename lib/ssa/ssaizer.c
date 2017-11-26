#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssaize-stmt.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/tree/tree-module.h"

DSEQ_GEN(htab, htab);

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        self->function = NULL;

        self->block = NULL;
        ssa_init_builder(&self->builder, self->context, NULL);

        allocator* alloc = ssa_get_alloc(context);
        dseq_init_ex_htab(&self->defs, alloc);
        htab_init_ex_ptr(&self->labels, alloc);
        dseq_init_ex_ptr(&self->continue_stack, alloc);
        dseq_init_ex_ptr(&self->break_stack, alloc);
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

static inline htab* ssaizer_get_last_scope(const ssaizer* self)
{
        S_ASSERT(dseq_size(&self->defs));
        return dseq_end_htab(&self->defs) - 1;
}

extern void ssaizer_push_scope(ssaizer* self)
{
        dseq_resize(&self->defs, dseq_size(&self->defs) + 1);
        htab_init_ex_ptr(ssaizer_get_last_scope(self),
                ssa_get_alloc(self->context));
}

extern void ssaizer_pop_scope(ssaizer* self)
{
        htab_dispose(ssaizer_get_last_scope(self));
        dseq_resize(&self->defs, dseq_size(&self->defs) - 1);
}

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* def)
{
        S_ASSERT(def);
        hiter it;
        htab* last = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        S_ASSERT(!htab_find(last, id, &it));
        htab_insert_ptr(last, id, def);
}

extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var)
{
        htab* first = dseq_begin_htab(&self->defs);
        htab* it = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);
        hiter res;

        while (it >= first)
        {
                if (htab_find(it, id, &res))
                        return hiter_get_ptr(&res);
                it--;
        }
        return NULL;
}

extern ssa_block* ssaizer_get_label_block(ssaizer* self, const tree_decl* label)
{
        tree_id id = tree_get_decl_name(label);
        hiter res;
        if (htab_find(&self->labels, id, &res))
                return hiter_get_ptr(&res);

        ssa_block* b = ssaizer_new_block(self);
        htab_insert_ptr(&self->labels, id, b);
        return b;
}

extern void ssaizer_push_continue_dest(ssaizer* self, ssa_block* block)
{
        dseq_append_ptr(&self->continue_stack, block);
}

extern void ssaizer_push_break_dest(ssaizer* self, ssa_block* block)
{
        dseq_append_ptr(&self->break_stack, block);
}

extern void ssaizer_pop_continue_dest(ssaizer* self)
{
        ssize size = dseq_size(&self->continue_stack);
        S_ASSERT(size);
        dseq_resize(&self->continue_stack, size - 1);
}

extern void ssaizer_pop_break_dest(ssaizer* self)
{
        ssize size = dseq_size(&self->break_stack);
        S_ASSERT(size);
        dseq_resize(&self->break_stack, size - 1);
}

extern ssa_block* ssaizer_get_continue_dest(ssaizer* self)
{
        return dseq_last_ptr(&self->continue_stack);
}

extern ssa_block* ssaizer_get_break_dest(ssaizer* self)
{
        return dseq_last_ptr(&self->break_stack);
}

static bool ssaizer_maybe_insert_return(ssaizer* self)
{
        if (!self->block)
        {
                self->block = ssaizer_new_block(self);
                ssaizer_enter_block(self, self->block);
        }

        if (ssa_get_block_exit(self->block))
                return true;

        tree_type* restype = tree_get_function_type_result(
                tree_get_decl_type(ssa_get_function_entity(self->function)));

        ssa_value* val = NULL;
        if (!tree_type_is_void(restype))
        {
                ssa_value* alloca = ssa_build_alloca(&self->builder, restype);
                val = ssa_build_load(&self->builder, alloca);
        }

        ssa_build_return(&self->builder, val);
        ssaizer_finish_current_block(self);
        return true;
}

static void ssaizer_function_cleanup(ssaizer* self)
{
        htab_clear(&self->labels);
}

static ssa_function* ssaize_function(ssaizer* self, tree_decl* func)
{
        tree_stmt* body = tree_get_function_body(func);
        if (!body)
                return NULL;

        self->function = ssa_new_function(self->context, func);
        ssa_builder_set_uid(&self->builder, 0);

        if (!ssaize_stmt(self, body))
                goto error;
        if (!ssaizer_maybe_insert_return(self))
                goto error;

        ssaizer_function_cleanup(self);
        ssa_fix_function_content_uids(self->function);
        return self->function;

error:
        ssaizer_function_cleanup(self);
        return NULL;
}

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module)
{
        ssa_module* m = ssa_new_module(self->context);
        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_FOREACH_DECL_IN_SCOPE(globals, func)
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