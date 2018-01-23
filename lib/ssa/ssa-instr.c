#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

DSEQ_GEN(value_use, ssa_value_use);

extern ssa_instr* ssa_new_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssize reserved_operands,
        ssize size)
{
        ssa_instr* instr = ssa_allocate(context, size);
        if (!instr)
                return NULL;

        ssa_init_variable(ssa_get_instr_var(instr), id, type);
        ssa_set_instr_kind(instr, kind);

        dseq* ops = &_ssa_instr_base(instr)->_operands;
        dseq_init_ex_value_use(ops, ssa_get_alloc(context));
        dseq_reserve(ops, reserved_operands);

        list_node_init(&_ssa_instr_base(instr)->_node);
        return instr;
}

extern ssa_instr* ssa_new_unary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        ssa_id id,
        tree_type* type,
        ssa_value* operand,
        ssize size)
{
        ssa_instr* instr = ssa_new_instr(context, kind, id, type, 1, size);
        if (!instr)
                return NULL;

        ssa_add_instr_operand(instr, operand);
        return instr;
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
        ssa_instr* instr = ssa_new_instr(context, kind, id, type, 2, size);
        if (!instr)
                return NULL;

        ssa_add_instr_operand(instr, first);
        ssa_add_instr_operand(instr, second);
        return instr;
}

static void ssa_init_value_use(ssa_value_use* self, ssa_instr* user)
{
        self->_instr = user;
        self->_value = NULL;
        list_node_init(&self->_node);
}

static void ssa_remove_value_use(ssa_value_use* self)
{
        self->_value = NULL;
        list_node_remove(&self->_node);
}

static void ssa_add_value_use(ssa_value* value, ssa_value_use* use)
{
        S_ASSERT(use->_value == NULL);
        use->_value = value;
        list_push_back(&_ssa_value_base(value)->_use_list, &use->_node);
}

extern ssa_value_use* ssa_add_instr_operand(ssa_instr* self, ssa_value* value)
{
        SSA_FOREACH_VALUE_USE(value, it, end)
                ;

        dseq* ops = &_ssa_instr_base(self)->_operands;
        ssize num_ops = dseq_size(ops);
        if (num_ops + 1 < dseq_total(ops))
                dseq_resize(ops, num_ops + 1);
        else 
        {
                dseq new_ops;
                dseq_init_ex(&new_ops, dseq_obsize(ops), dseq_alloc(ops));
                dseq_resize(&new_ops, num_ops + 1);
                //memcpy(dseq_begin_value_use(&new_ops),
                //        dseq_begin_value_use(ops), sizeof(ssa_value_use) * num_ops);

                for (ssize i = 0; i < num_ops; i++)
                {
                        ssa_value_use* op = dseq_begin_value_use(ops) + i;
                        ssa_value_use* new_op = dseq_begin_value_use(&new_ops) + i;
                        *new_op = *op;
                        op->_node._prev->_next = &new_op->_node;
                        op->_node._next->_prev = &new_op->_node;
                }

                dseq_dispose(ops);
                dseq_move(ops, &new_ops);
        }
        ssa_value_use* last = dseq_end_value_use(ops) - 1;
        ssa_init_value_use(last, self);
        ssa_add_value_use(value, last);
        return last;
}

extern bool ssa_instr_has_var(const ssa_instr* self)
{
        switch (ssa_get_instr_kind(self))
        {
                case SIK_STORE:
                case SIK_TERMINATOR:
                        return false;

                case SIK_CALL:
                        return !ssa_call_returns_void(self);

                default:
                        return true;
        }
}

extern ssa_value* ssa_get_instr_operand_value(const ssa_instr* self, size_t i)
{
        return ssa_get_value_use_value(ssa_get_instr_operand(self, i));
}

extern ssa_value_use* ssa_get_instr_operand(const ssa_instr* self, size_t i)
{
        const dseq* ops = &_ssa_instr_cbase(self)->_operands;
        S_ASSERT(i < dseq_size(ops));
        return dseq_begin_value_use(ops) + i;
}

extern ssa_value_use* ssa_get_instr_operands_begin(const ssa_instr* self)
{
        return dseq_begin_value_use(&_ssa_instr_cbase(self)->_operands);
}

extern ssa_value_use* ssa_get_instr_operands_end(const ssa_instr* self)
{
        return dseq_end_value_use(&_ssa_instr_cbase(self)->_operands);
}

extern ssize ssa_get_instr_num_operands(const ssa_instr* self)
{
        return dseq_size(&_ssa_instr_cbase(self)->_operands);
}

extern void ssa_remove_instr(ssa_instr* self)
{
        if (ssa_instr_has_var(self))
                S_ASSERT(!ssa_value_is_used(ssa_get_instr_cvar(self))
                        && "Cannot remove used instruction");

        SSA_FOREACH_INSTR_OPERAND(self, it, end)
                ssa_remove_value_use(it);

        list_node_remove(&_ssa_instr_base(self)->_node);
        ssa_set_instr_kind(self, SIK_INVALID);
}

extern void ssa_set_instr_operand_value(ssa_instr* self, size_t i, ssa_value* val)
{
        ssa_value_use* op = ssa_get_instr_operand(self, i);
        ssa_remove_value_use(op);
        ssa_add_value_use(val, op);
}

extern ssa_instr* ssa_new_alloca(ssa_context* context, ssa_id id, tree_type* type)
{
        return ssa_new_instr(context, SIK_ALLOCA, id, type, 0, sizeof(struct _ssa_alloca));
}

extern tree_type* ssa_get_allocated_type(const ssa_instr* self)
{
        return tree_get_pointer_target(ssa_get_value_type(ssa_get_instr_cvar(self)));
}

extern ssa_instr* ssa_new_load(
        ssa_context* context, ssa_id id, tree_type* type, ssa_value* what)
{
        return ssa_new_unary_instr(
                context, SIK_LOAD, id, type, what, sizeof(struct _ssa_load));
}

extern ssa_instr* ssa_new_cast(
        ssa_context* context, ssa_id id, tree_type* type, ssa_value* operand)
{
        return ssa_new_unary_instr(
                context, SIK_CAST, id, type, operand, sizeof(struct _ssa_load));
}

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        ssa_id id,
        tree_type* restype,
        ssa_binop_kind kind,
        ssa_value* lhs,
        ssa_value* rhs)
{
        ssa_instr* binop = ssa_new_binary_instr(
                context, SIK_BINARY, id, restype, lhs, rhs, sizeof(struct _ssa_binop));
        if (!binop)
                return NULL;

        ssa_set_binop_kind(binop, kind);
        return binop;
}

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where)
{
        return ssa_new_binary_instr(
                context, SIK_STORE, 0, NULL, what, where, sizeof(struct _ssa_store));
}

extern ssa_instr* ssa_new_getfieldaddr(
        ssa_context* context,
        ssa_id id,
        tree_type* type,
        ssa_value* operand,
        unsigned field_index)
{
        ssa_instr* gfa = ssa_new_unary_instr(
                context, SIK_GETFIELDADDR, id, type, operand, sizeof(struct _ssa_getfieldaddr));
        if (!gfa)
                return NULL;

        ssa_set_getfieldaddr_index(gfa, field_index);
        return gfa;
}

extern ssa_instr* ssa_new_call(ssa_context* context, ssa_id id, tree_type* type, ssa_value* func)
{
        return ssa_new_unary_instr(context, SIK_CALL, id, type, func, sizeof(struct _ssa_call));
}

extern bool ssa_call_returns_void(const ssa_instr* self)
{
        ssa_value* func = ssa_get_called_func(self);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));
        return tree_type_is_void(tree_get_function_type_result(func_type));
}

extern ssa_instr* ssa_new_phi(ssa_context* context, ssa_id id, tree_type* restype)
{
        return ssa_new_instr(context, SIK_PHI, id, restype, 0, sizeof(struct _ssa_phi));
}

extern void ssa_add_phi_operand(
        ssa_instr* self, ssa_context* context, ssa_value* value, ssa_value* label)
{
        ssa_add_instr_operand(self, value);
        ssa_add_instr_operand(self, label);
}

extern ssa_instr* ssa_new_terminator_instr(
        ssa_context* context,
        ssa_terminator_instr_kind kind,
        ssize reserved_operands,
        ssize size)
{
        ssa_instr* instr = ssa_new_instr(context,
                SIK_TERMINATOR, 0, NULL, reserved_operands, size);
        if (!instr)
                return NULL;

        ssa_set_terminator_instr_kind(instr, kind);
        return instr;
}

extern ssa_value_use* ssa_get_terminator_instr_successors_begin(const ssa_instr* self)
{
        switch (ssa_get_terminator_instr_kind(self))
        {
                case STIK_INDERECT_JUMP:
                        return ssa_get_instr_operands_begin(self);

                case STIK_CONDITIONAL_JUMP:
                case STIK_SWITCH:
                        return ssa_get_instr_operands_begin(self) + 1;

                case STIK_RETURN:
                        return ssa_get_instr_operands_end(self);

                default:
                        S_UNREACHABLE();
                        return NULL;
        }
}

extern ssa_instr* ssa_new_inderect_jump(ssa_context* context, ssa_value* dest)
{
        ssa_instr* jump = ssa_new_unary_instr(context, 
                SIK_TERMINATOR, 0, NULL, dest, sizeof(struct _ssa_inderect_jump));
        if (!jump)
                return NULL;

        ssa_set_terminator_instr_kind(jump, STIK_INDERECT_JUMP);
        return jump;
}

extern ssa_instr* ssa_new_conditional_jump(
        ssa_context* context, ssa_value* condition, ssa_value* on_true, ssa_value* on_false)
{
        ssa_instr* jump = ssa_new_terminator_instr(context, 
                STIK_CONDITIONAL_JUMP, 3, sizeof(struct _ssa_conditional_jump));
        if (!jump)
                return NULL;

        ssa_add_instr_operand(jump, condition);
        ssa_add_instr_operand(jump, on_true);
        ssa_add_instr_operand(jump, on_false);
        return jump;
}

extern ssa_instr* ssa_new_switch_instr(ssa_context* context, ssa_value* condition)
{
        ssa_instr* jump = ssa_new_unary_instr(context,
                SIK_TERMINATOR, 0, NULL, condition, sizeof(struct _ssa_switch_instr));
        if (!jump)
                return NULL;

        ssa_set_terminator_instr_kind(jump, STIK_SWITCH);
        return jump;
}

extern void ssa_add_switch_case(ssa_instr* self,
        ssa_context* context, ssa_value* value, ssa_value* label)
{
        ssa_add_instr_operand(self, value);
        ssa_add_instr_operand(self, label);
}

extern ssa_instr* ssa_new_ret_void(ssa_context* context)
{
        return ssa_new_terminator_instr(
                context, STIK_RETURN, 0, sizeof(struct _ssa_ret_instr));
}

extern ssa_instr* ssa_new_ret(ssa_context* context, ssa_value* value)
{
        ssa_instr* ret = ssa_new_terminator_instr(
                context, STIK_RETURN, 1, sizeof(struct _ssa_ret_instr));
        if (!ret)
                return NULL;

        ssa_add_instr_operand(ret, value);
        return ret;
}