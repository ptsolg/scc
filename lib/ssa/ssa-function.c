#include "scc/ssa/ssa-function.h"
#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

extern ssa_function* ssa_new_function(ssa_context* context, tree_decl* func)
{
        ssa_function* f = ssa_allocate(context, sizeof(ssa_function));
        if (!f)
                return NULL;

        ssa_set_function_entity(f, func);
        list_init(&f->_blocks);
        list_node_init(&f->_node);
        dseq_init_ex_ptr(&f->_params, ssa_get_alloc(context));
        return f;
}

extern void ssa_add_function_block(ssa_function* self, ssa_block* block)
{
        S_ASSERT(block);
        list_push_back(&self->_blocks, &block->_node);
}

extern void ssa_add_function_param(ssa_function* self, ssa_value* param)
{
        S_ASSERT(param);
        dseq_append_ptr(&self->_params, param);
}

extern bool ssa_function_returns_void(const ssa_function* self)
{
        return tree_type_is_void(ssa_get_function_result_type(self));
}

extern void ssa_fix_function_content_uids(ssa_function* self)
{
        ssa_id uid = 0;
        SSA_FOREACH_FUNCTION_PARAM(self, it)
                ssa_set_value_id(*it, uid++);

        SSA_FOREACH_FUNCTION_BLOCK(self, block)
        {
                ssa_set_value_id(ssa_get_block_label(block), uid++);
                SSA_FOREACH_BLOCK_INSTR(block, instr)
                        if (ssa_instr_has_var(instr))
                                ssa_set_value_id(ssa_get_instr_var(instr), uid++);
        }
}