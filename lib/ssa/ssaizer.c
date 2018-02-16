#include "scc/ssa/ssaizer.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-module.h"
#include "scc/ssa/ssaize-decl.h"
#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/tree/tree-module.h"
#include "scc/scl/dseq-instance.h"

#define DSEQ_VALUE_TYPE strmap
#define DSEQ_TYPE strmap_stack
#define DSEQ_INIT strmap_stack_init
#define DSEQ_INIT_ALLOC strmap_stack_init_alloc
#define DSEQ_DISPOSE strmap_stack_dispose
#define DSEQ_GET_SIZE strmap_stack_size
#define DSEQ_GET_CAPACITY strmap_stack_capacity
#define DSEQ_GET_ALLOCATOR strmap_stack_allocator
#define DSEQ_RESERVE strmap_stack_reserve
#define DSEQ_RESIZE strmap_stack_resize
#define DSEQ_GET_BEGIN strmap_stack_begin
#define DSEQ_GET_END strmap_stack_end
#define DSEQ_GET strmap_stack_get
#define DSEQ_SET strmap_stack_set
#define DSEQ_APPEND strmap_stack_append

#include "scc/scl/dseq.h"

#undef DSEQ_VALUE_TYPE
#undef DSEQ_TYPE 
#undef DSEQ_INIT 
#undef DSEQ_INIT_ALLOC 
#undef DSEQ_DISPOSE 
#undef DSEQ_GET_SIZE 
#undef DSEQ_GET_CAPACITY 
#undef DSEQ_GET_ALLOCATOR 
#undef DSEQ_RESERVE 
#undef DSEQ_RESIZE 
#undef DSEQ_GET_BEGIN 
#undef DSEQ_GET_END 
#undef DSEQ_GET 
#undef DSEQ_SET 
#undef DSEQ_APPEND

extern void ssaizer_init(ssaizer* self, ssa_context* context)
{
        self->context = context;
        self->function = NULL;
        self->module = NULL;

        self->block = NULL;
        ssa_init_builder(&self->builder, self->context, NULL);

        allocator* alloc = ssa_get_alloc(context);
        strmap_stack_init(&self->defs);
        strmap_init_alloc(&self->labels, alloc);
        strmap_init_alloc(&self->globals, alloc);
        dseq_init_alloc(&self->continue_stack, alloc);
        dseq_init_alloc(&self->break_stack, alloc);
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

static inline strmap* ssaizer_get_last_scope(const ssaizer* self)
{
        S_ASSERT(dseq_size(&self->defs));
        return strmap_stack_end(&self->defs) - 1;
}

extern void ssaizer_push_scope(ssaizer* self)
{
        strmap_stack_resize(&self->defs, strmap_stack_size(&self->defs) + 1);
        strmap_init_alloc(ssaizer_get_last_scope(self), ssa_get_alloc(self->context));
}

extern void ssaizer_pop_scope(ssaizer* self)
{
        strmap_dispose(ssaizer_get_last_scope(self));
        strmap_stack_resize(&self->defs, strmap_stack_size(&self->defs) - 1);
}

extern void ssaizer_set_def(ssaizer* self, const tree_decl* var, ssa_value* def)
{
        S_ASSERT(def);
        strmap_iter it;
        strmap* last = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);

        S_ASSERT(!strmap_find(last, id, &it));
        strmap_insert(last, id, def);
}

extern ssa_value* ssaizer_get_def(ssaizer* self, const tree_decl* var)
{
        strmap* first = strmap_stack_begin(&self->defs);
        strmap* it = ssaizer_get_last_scope(self);
        tree_id id = tree_get_decl_name(var);
        strmap_iter res;

        while (it >= first)
        {
                if (strmap_find(it, id, &res))
                        return *strmap_iter_value(&res);
                it--;
        }
        return ssaizer_get_global_decl(self, var);
}

extern void ssaizer_set_global_decl(ssaizer* self, const tree_decl* var, ssa_value* decl)
{
        S_ASSERT(decl);
        strmap_insert(&self->globals, tree_get_decl_name(var), decl);
}

extern ssa_value* ssaizer_get_global_decl(ssaizer* self, const tree_decl* var)
{
        strmap_iter res;
        return strmap_find(&self->globals, tree_get_decl_name(var), &res)
                ? *strmap_iter_value(&res)
                : NULL;
}

extern ssa_block* ssaizer_get_label_block(ssaizer* self, const tree_decl* label)
{
        tree_id id = tree_get_decl_name(label);
        strmap_iter res;
        if (strmap_find(&self->labels, id, &res))
                return *strmap_iter_value(&res);

        ssa_block* b = ssaizer_new_block(self);
        strmap_insert(&self->labels, id, b);
        return b;
}

extern void ssaizer_push_continue_dest(ssaizer* self, ssa_block* block)
{
        dseq_append(&self->continue_stack, block);
}

extern void ssaizer_push_break_dest(ssaizer* self, ssa_block* block)
{
        dseq_append(&self->break_stack, block);
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
        return *(dseq_end(&self->continue_stack) - 1);
}

extern ssa_block* ssaizer_get_break_dest(ssaizer* self)
{
        return *(dseq_end(&self->break_stack) - 1);
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