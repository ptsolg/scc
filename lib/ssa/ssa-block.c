#include "scc/ssa/ssa-block.h"
#include "scc/ssa/ssa-context.h"

extern ssa_block* ssa_new_block(ssa_context* context, ssa_id entry_id, ssa_branch* exit)
{
        ssa_block* b = ssa_allocate(context, sizeof(*b));
        if (!b)
                return NULL;

        ssa_set_block_entry_id(b, entry_id);
        ssa_set_block_exit(b, exit);
        dseq_init_ex_ptr(&b->_values, ssa_get_context_alloc(context));
        return b;
}