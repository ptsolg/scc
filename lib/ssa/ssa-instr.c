#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

extern ssa_instr* ssa_new_instr(
        ssa_context* context, ssa_instr_kind kind, ssa_id id, tree_type* type, ssize size)
{
        ssa_instr* i = ssa_allocate(context, size);
        if (!i)
                return NULL;


        ssa_set_instr_kind(i, kind);
        list_node_init(&_ssa_get_instr_base(i)->_node);
        ssa_init_variable(ssa_get_instr_var(i), id, type);
        return i;
}

extern bool ssa_instr_has_var(const ssa_instr* self)
{
        ssa_instr_kind k = ssa_get_instr_kind(self);
        return k != SIK_STORE && (k != SIK_CALL || !ssa_call_returns_void(self));
}

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binary_instr_kind opcode,
        ssa_value* lhs,
        ssa_value* rhs)
{
        ssa_instr* i = ssa_new_instr(context,
                SIK_BINARY, id, restype, sizeof(struct _ssa_binary_instr));
        if (!i)
                return NULL;

        ssa_set_binop_opcode(i, opcode);
        ssa_set_binop_lhs(i, lhs);
        ssa_set_binop_rhs(i, rhs);
        return i;
}

extern ssa_instr* ssa_new_cast(ssa_context* context, ssa_id id, tree_type* type, ssa_value* operand)
{
        ssa_instr* i = ssa_new_instr(context, SIK_CAST, id, type, sizeof(struct _ssa_cast_instr));
        if (!i)
                return NULL;

        ssa_set_cast_operand(i, operand);
        return i;
}


extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, ssa_value* func)
{
        ssa_instr* i = ssa_new_instr(context,
                SIK_CALL, id, NULL, sizeof(struct _ssa_call_instr));
        if (!i)
                return NULL;

        ssa_set_called_func(i, func);
        dseq_init_ex_ptr(&_ssa_get_call(i)->_args, ssa_get_alloc(context));
        return i;
}

extern void ssa_set_call_args(ssa_instr* self, dseq* args)
{
        S_ASSERT(args);
        dseq_move(&_ssa_get_call(self)->_args, args);
}

extern void ssa_add_call_arg(ssa_instr* self, ssa_value* arg)
{
        S_ASSERT(arg);
        dseq_append_ptr(&_ssa_get_call(self)->_args, arg);
}

extern bool ssa_call_returns_void(const ssa_instr* self)
{
        ssa_value* func = ssa_get_called_func(self);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));
        return tree_type_is_void(tree_get_function_type_result(func_type));
}

extern ssa_instr* ssa_new_getaddr(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_value* pointer,
        ssa_value* index,
        ssa_value* offset)
{
        ssa_instr* i = ssa_new_instr(context, SIK_GETADDR, id, restype, sizeof(struct _ssa_getaddr_instr));
        if (!i)
                return NULL;

        ssa_set_getaddr_operand(i, pointer);
        ssa_set_getaddr_index(i, index);
        ssa_set_getaddr_offset(i, offset);
        return i;
}

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype)
{
        ssa_instr* i = ssa_new_instr(context, SIK_PHI, id, restype, sizeof(struct _ssa_phi_instr));
        if (!i)
                return NULL;

        dseq_init_ex_ptr(&_ssa_get_phi(i)->_args, ssa_get_alloc(context));
        return i;
}

extern void ssa_add_phi_arg(ssa_instr* self, ssa_value* arg)
{
        S_ASSERT(arg);
        dseq_append_ptr(&_ssa_get_phi(self)->_args, arg);
}

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type)
{
       return ssa_new_instr(context, SIK_ALLOCA, id, type, sizeof(struct _ssa_alloca_instr));
}

extern ssa_instr* ssa_new_load(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* what)
{
        ssa_instr* i = ssa_new_instr(context, SIK_LOAD, id, type, sizeof(struct _ssa_load_instr));
        if (!i)
                return NULL;

        ssa_set_load_what(i, what);
        return i;
}

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where)
{
        ssa_instr* i = ssa_new_instr(context, SIK_STORE, 0, NULL, sizeof(struct _ssa_store_instr));
        if (!i)
                return NULL;

        ssa_set_store_what(i, what);
        ssa_set_store_where(i, where);
        return i;
}