#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"

extern ssa_block* ssa_new_block(ssa_context* context, ssa_id entry_id, ssa_branch* exit)
{
        ssa_block* b = ssa_allocate(context, sizeof(*b));
        if (!b)
                return NULL;

        ssa_init_label(ssa_get_block_value(b), entry_id);
        ssa_set_block_exit(b, exit);
        list_init(&b->_instr_list);
        list_node_init(&b->_node);
        return b;
}

extern void ssa_add_block_instr(ssa_block* self, ssa_instr* i)
{
        S_ASSERT(i);
        list_push_back(&self->_instr_list, (list_node*)i);
}

extern ssa_instr* ssa_block_get_first_phi(const ssa_block* self)
{
        SSA_FOREACH_BLOCK_INSTR(self, instr)
                if (ssa_get_instr_kind(instr) == SIK_PHI)
                        return instr;

        return NULL;
}