#include "scc/ssa/ssa-opt.h"
#include "scc/ssa/ssa-block.h"

static void ssa_constant_fold_conditional_jumps(ssa_context* context, ssa_value* func)
{
        SSA_FOREACH_FUNCTION_BLOCK(func, block)
        {
                ssa_instr* cond_jump = ssa_get_block_terminator(block);
                if (!cond_jump
                        || ssa_value_is_used(ssa_get_block_label(block))
                        || ssa_get_terminator_instr_kind(cond_jump) != STIK_CONDITIONAL_JUMP)
                {
                        continue;
                }

                ssa_value* cond = ssa_get_instr_operand_value(cond_jump, 0);
                if (ssa_get_value_kind(cond) != SVK_CONSTANT)
                        continue;

                bool cond_val = avalue_is_zero(ssa_get_constant_cvalue(cond));
                ssa_value* live_branch = ssa_get_instr_operand_value(cond_jump, cond_val ? 2 : 1);
                ssa_instr* inderect_jump = ssa_new_inderect_jump(context, live_branch);

                ssa_add_instr_after(inderect_jump, cond_jump);
                ssa_remove_instr(cond_jump);
        }
}

static void ssa_remove_unused_block(ssa_block* block)
{
        if (ssa_value_is_used(ssa_get_block_label(block)))
                return;

        // todo: replace values from unreachable block with SVK_UNDEF and remove block
        SSA_FOREACH_BLOCK_INSTR(block, instr)
                if (ssa_instr_has_var(instr) && ssa_value_is_used(ssa_get_instr_var(instr)))
                        return;

        ssa_remove_block(block);
}

static void ssa_remove_unused_blocks(ssa_value* func)
{
        ssa_block* it = ssa_get_function_blocks_begin(func);
        ssa_block* end = ssa_get_function_blocks_end(func);
        if (it == end)
                return;

        it = ssa_get_next_block(it); // skip entry
        while (it != end)
        {
                ssa_block* next = ssa_get_next_block(it);
                ssa_remove_unused_block(it);
                it = next;
        }
}

extern void ssa_eliminate_dead_code(const ssa_pass* pass)
{
        ssa_constant_fold_conditional_jumps(pass->context, pass->function);
        ssa_remove_unused_blocks(pass->function);
}