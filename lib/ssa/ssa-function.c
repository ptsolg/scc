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
        return f;
}

extern void ssa_add_function_block(ssa_function* self, ssa_block* block)
{
        S_ASSERT(block);
        list_push_back(&self->_blocks, &block->_node);
}

extern void ssa_fix_function_content_uids(ssa_function* self)
{
        ssa_id uid = 0;
        SSA_FOREACH_FUNCTION_BLOCK(self, block)
        {
                ssa_set_value_id(ssa_get_block_value(block), uid++);
                SSA_FOREACH_BLOCK_INSTR(block, instr)
                {
                        if (ssa_get_instr_kind(instr) == SIK_STORE)
                                continue;
                        ssa_set_value_id(ssa_get_instr_value(instr), uid++);
                }
        }
}