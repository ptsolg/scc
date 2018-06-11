#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-context.h"
#include "scc/ssa/ssa-instr.h"

extern ssa_block* ssa_new_block(ssa_context* context, bool atomic)
{
        ssa_block* b = ssa_allocate_node(context, sizeof(*b));
        if (!b)
                return NULL;

        ssa_init_label(ssa_get_block_label(b), ssa_get_type_for_label(context));

        b->atomic = atomic;
        _ssa_init_instr_node(&b->instr_node, b, true);
        list_node_init(&b->node);
        return b;
}

extern void ssa_move_block_instrs(ssa_block* from, ssa_block* to)
{
        SSA_FOREACH_BLOCK_INSTR_SAFE(from, it, next)
                ssa_move_instr(it, ssa_get_block_instrs_end(to), false);
}

extern void ssa_add_block_after(ssa_block* block, ssa_block* pos)
{
        list_node_add_after(&pos->node, &block->node);
}

extern void ssa_add_block_before(ssa_block* block, ssa_block* pos)
{
        list_node_add_before(&pos->node, &block->node);
}

extern ssa_instr* ssa_block_get_first_phi(const ssa_block* self)
{
        SSA_FOREACH_BLOCK_INSTR(self, instr)
                if (ssa_get_instr_kind(instr) == SIK_PHI)
                        return instr;

        return NULL;
}