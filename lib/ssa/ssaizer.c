#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/tree/tree-module.h"

DSEQ_GEN(htab, htab);

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        self->function = NULL;
        self->module = NULL;

        self->block = NULL;
        ssa_init_builder(&self->builder, self->context, NULL);

        allocator* alloc = ssa_get_alloc(context);
        dseq_init_ex_htab(&self->defs, alloc);
        htab_init_ex_ptr(&self->labels, alloc);
        htab_init_ex_ptr(&self->globals, alloc);
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
        ssa_builder_set_pos(&self->builder, ssa_get_block_instrs_end(self->block), false);
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
        return ssa_new_block(self->context, ssa_builder_gen_uid(&self->builder));
}

extern bool ssaizer_current_block_is_terminated(const ssaizer* self)
{
        S_ASSERT(self->block);
        ssa_instr* terminator = ssa_get_block_terminator(self->block);
        return terminator && ssa_get_instr_kind(terminator) == SIK_TERMINATOR;
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
        return ssaizer_get_global_decl(self, var);
}

extern void ssaizer_set_global_decl(ssaizer* self, const tree_decl* var, ssa_value* decl)
{
        S_ASSERT(decl);
        htab_insert_ptr(&self->globals, tree_get_decl_name(var), decl);
}

extern ssa_value* ssaizer_get_global_decl(ssaizer* self, const tree_decl* var)
{
        hiter res;
        return htab_find(&self->globals, tree_get_decl_name(var), &res)
                ? hiter_get_ptr(&res)
                : NULL;
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

extern ssa_module* ssaize_module(ssaizer* self, const tree_module* module)
{
        self->module = ssa_new_module(self->context);
        const tree_decl_scope* globals = tree_get_module_cglobals(module);
        TREE_FOREACH_DECL_IN_SCOPE(globals, decl)
        {
                if (!tree_decl_is(decl, TDK_FUNCTION))
                        continue;
                if (!ssaize_function_decl(self, decl))
                        return NULL;
        }
        return self->module;
}