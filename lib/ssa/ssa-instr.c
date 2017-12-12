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

extern bool ssa_instr_is_unary(const ssa_instr* self)
{
        return ssa_get_instr_kind(self) == SIK_CAST
                || ssa_get_instr_kind(self) == SIK_LOAD;
}

extern bool ssa_instr_is_binary(const ssa_instr* self)
{
        return ssa_get_instr_kind(self) == SIK_BINARY
                || ssa_get_instr_kind(self) == SIK_STORE;
}
extern bool ssa_instr_is_ternary(const ssa_instr* self)
{
        return ssa_get_instr_kind(self) == SIK_GETADDR;
}

extern bool ssa_instr_is_nary(const ssa_instr* self)
{
        return ssa_get_instr_kind(self) == SIK_CALL
                || ssa_get_instr_kind(self) == SIK_PHI;
}

extern bool ssa_instr_has_var(const ssa_instr* self)
{
        ssa_instr_kind k = ssa_get_instr_kind(self);
        return k != SIK_STORE && (k != SIK_CALL || !ssa_call_returns_void(self));
}

extern ssa_value** ssa_get_instr_operands_begin(ssa_instr* self)
{
        if (ssa_instr_is_binary(self))
                return ssa_get_binary_instr_operands_begin(self);
        else if (ssa_instr_is_unary(self))
                return ssa_get_unary_instr_poperand(self);
        else if (ssa_instr_is_nary(self))
                return ssa_get_nary_instr_operands_begin(self);
        else if (ssa_instr_is_ternary(self))
                return ssa_get_ternary_instr_operands_begin(self);
        return NULL;
}

extern ssa_value** ssa_get_instr_operands_end(ssa_instr* self)
{
        if (ssa_instr_is_binary(self))
                return ssa_get_binary_instr_operands_end(self);
        else if (ssa_instr_is_unary(self))
                return ssa_get_unary_instr_poperand(self) + 1;
        else if (ssa_instr_is_nary(self))
                return ssa_get_nary_instr_operands_end(self);
        else if (ssa_instr_is_ternary(self))
                return ssa_get_ternary_instr_operands_end(self);
        return NULL;
}

extern ssa_instr* ssa_new_unary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* operand)
{
        ssa_instr* i = ssa_new_instr(context, kind, id, type, sizeof(struct _ssa_unary_instr));
        if (!i)
                return NULL;

        ssa_set_unary_instr_operand(i, operand);
        return i;
}

extern ssa_instr* ssa_new_binary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        ssize size)
{
        ssa_instr* i = ssa_new_instr(context, kind, id, type, size);
        if (!i)
                return NULL;

        ssa_get_binary_instr_operands_begin(i)[0] = first;
        ssa_get_binary_instr_operands_begin(i)[1] = second;
        return i;
}

extern ssa_instr* ssa_new_ternary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        ssa_value* third)
{
        ssa_instr* i = ssa_new_instr(context, kind, id, type,
                sizeof(struct _ssa_ternary_instr));
        if (!i)
                return NULL;

        ssa_get_ternary_instr_operands_begin(i)[0] = first;
        ssa_get_ternary_instr_operands_begin(i)[1] = second;
        ssa_get_ternary_instr_operands_begin(i)[2] = third;
        return i;
}

extern ssa_instr* ssa_new_nary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type)
{
        ssa_instr* i = ssa_new_instr(context, kind, id, type,
                sizeof(struct _ssa_nary_instr));
        if (!i)
                return NULL;

        dseq_init_ex_ptr(&_ssa_get_nary_instr(i)->_operands, ssa_get_alloc(context));
        return i;
}

extern void ssa_add_nary_instr_operand(ssa_instr* self, ssa_value* operand)
{
        S_ASSERT(operand);
        dseq_append_ptr(&_ssa_get_nary_instr(self)->_operands, operand);
}

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binary_instr_kind opcode,
        ssa_value* lhs,
        ssa_value* rhs)
{
        ssa_instr* i = ssa_new_binary_instr(context,
                SIK_BINARY, id, restype, lhs, rhs, sizeof(struct _ssa_binop_instr));
        if (!i)
                return NULL;

        ssa_set_binop_kind(i, opcode);
        return i;
}

extern ssa_instr* ssa_new_cast(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* operand)
{
        return ssa_new_unary_instr(context, SIK_CAST, id, type, operand);
}

extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, ssa_value* func)
{
        ssa_instr* i = ssa_new_nary_instr(context, SIK_CALL, id, NULL);
        if (!i)
                return NULL;

        ssa_add_nary_instr_operand(i, func);
        ssa_set_called_func(i, func);
        return i;
}

extern void ssa_add_call_arg(ssa_instr* self, ssa_value* arg)
{
        SSA_ASSERT_INSTR(self, SIK_CALL);
        ssa_add_nary_instr_operand(self, arg);
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
        return ssa_new_ternary_instr(context,
                SIK_GETADDR, id, restype, pointer, index, offset);
}

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype)
{
        return ssa_new_nary_instr(context, SIK_PHI, id, restype);
}

extern void ssa_add_phi_arg(ssa_instr* self, ssa_value* arg)
{
        SSA_ASSERT_INSTR(self, SIK_PHI);
        ssa_add_nary_instr_operand(self, arg);
}

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type)
{
       return ssa_new_instr(context, SIK_ALLOCA, id, type, sizeof(struct _ssa_alloca_instr));
}

extern ssa_instr* ssa_new_load(ssa_context* context,
        ssa_id id, tree_type* type, ssa_value* what)
{
        return ssa_new_unary_instr(context, SIK_LOAD, id, type, what);
}

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where)
{
        return ssa_new_binary_instr(context,
                SIK_STORE, 0, NULL, what, where, sizeof(struct _ssa_store_instr));
}