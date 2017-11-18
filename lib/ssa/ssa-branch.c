#include "scc/ssa/ssa-branch.h"
#include "scc/ssa/ssa-context.h"

extern ssa_branch* ssa_new_branch(ssa_context* context, ssa_branch_kind kind, ssize size)
{
        ssa_branch* br = ssa_allocate(context, size);
        if (!br)
                return NULL;

        ssa_set_branch_kind(br, kind);
        return br;
}

extern ssa_branch* ssa_new_if_branch(
        ssa_context* context, ssa_value* cond, ssa_value* if_true, ssa_value* if_false)
{
        ssa_branch* br = ssa_new_branch(context, SBK_IF, sizeof(struct _ssa_if_branch));
        if (!br)
                return NULL;

        ssa_set_if_cond(br, cond);
        ssa_set_if_true_block(br, if_true);
        ssa_set_if_false_block(br, if_false);
        return br;
}

extern ssa_branch* ssa_new_return_branch(ssa_context* context, ssa_value* val)
{
        ssa_branch* br = ssa_new_branch(context, SBK_RETURN, sizeof(struct _ssa_return_branch));
        if (!br)
                return NULL;

        ssa_set_return_value(br, val);
        return br;
}

extern ssa_branch* ssa_new_jump_branch(ssa_context* context, ssa_value* dest)
{
        ssa_branch* br = ssa_new_branch(context, SBK_JUMP, sizeof(struct _ssa_jump_branch));
        if (!br)
                return NULL;

        ssa_set_jump_dest(br, dest);
        return br;
}