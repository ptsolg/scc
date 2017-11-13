#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

extern ssa_instr* ssa_new_instr(ssa_context* context, ssa_instr_kind kind, ssize size)
{
        ssa_instr* i = ssa_allocate(context, size);
        if (!i)
                return NULL;

        ssa_set_instr_kind(i, kind);
        return i;
}

extern ssa_instr* ssa_new_binop(
        ssa_context* context, ssa_binary_instr_kind opcode, ssa_value* lhs, ssa_value* rhs)
{
        ssa_instr* i = ssa_new_instr(context, SIK_BINARY, sizeof(struct _ssa_binary_instr));
        if (!i)
                return NULL;

        ssa_set_binop_opcode(i, opcode);
        ssa_set_binop_lhs(i, lhs);
        ssa_set_binop_rhs(i, rhs);
        return i;
}

extern ssa_instr* ssa_new_cast(ssa_context* context, tree_type* type, ssa_value* operand)
{
        ssa_instr* i = ssa_new_instr(context, SIK_CAST, sizeof(struct _ssa_cast_instr));
        if (!i)
                return NULL;

        ssa_set_cast_type(i, type);
        ssa_set_cast_operand(i, operand);
        return i;
}

extern ssa_instr* ssa_new_call(ssa_context* context, tree_decl* func)
{
        ssa_instr* i = ssa_new_instr(context, SIK_CALL, sizeof(struct _ssa_call_instr));
        if (!i)
                return NULL;

        ssa_set_call_func(i, func);
        dseq_init_ex_ptr(&_ssa_get_call(i)->_args, ssa_get_context_alloc(context));
        return i;
}

extern ssa_instr* ssa_new_getaddr(
        ssa_context* context, ssa_value* operand, ssize offset)
{
        ssa_instr* i = ssa_new_instr(context, SIK_GETADDR, sizeof(struct _ssa_getaddr_instr));
        if (!i)
                return NULL;

        ssa_set_getaddr_operand(i, operand);
        ssa_set_getaddr_offset(i, offset);
        return i;
}

extern ssa_instr* ssa_new_getptrval(
        ssa_context* context, ssa_value* pointer, ssize index, ssize offset)
{
        ssa_instr* i = ssa_new_instr(context, SIK_GETPTRVAL, sizeof(struct _ssa_getptrval_instr));
        if (!i)
                return NULL;

        ssa_set_getptrval_operand(i, pointer);
        ssa_set_getptrval_index(i, index);
        ssa_set_getptrval_offset(i, offset);
        return i;
}

extern ssa_instr* ssa_new_phi(ssa_context* context)
{
        ssa_instr* i = ssa_new_instr(context, SIK_PHI, sizeof(struct _ssa_phi_instr));
        if (!i)
                return NULL;

        // todo

        return i;
}

extern ssa_instr* ssa_new_init(ssa_context* context, ssa_value* operand)
{
        ssa_instr* i = ssa_new_instr(context, SIK_INIT, sizeof(struct _ssa_init_instr));
        if (!i)
                return NULL;

        ssa_set_init_value(i, operand);
        return i;
}