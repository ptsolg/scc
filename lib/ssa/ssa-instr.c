#include "scc/ssa/ssa-instr.h"
#include "scc/ssa/ssa-context.h"

extern void _ssa_init_instr_node(struct _ssa_instr_node* self, ssa_block* block, bool init_as_list)
{
        self->block = block;
        if (init_as_list)
                list_init(&self->list);
        else
                list_node_init(&self->node);
}

extern void _ssa_add_instr_node_after(struct _ssa_instr_node* self, struct _ssa_instr_node* pos)
{
        self->block = pos->block;
        list_node_add_after(&pos->node, &self->node);
}

extern void _ssa_add_instr_node_before(struct _ssa_instr_node* self, struct _ssa_instr_node* pos)
{
        self->block = pos->block;
        list_node_add_before(&pos->node, &self->node);
}

extern void _ssa_remove_instr_node(struct _ssa_instr_node* self)
{
        self->block = NULL;
        list_node_remove(&self->node);
}

extern ssa_instr* ssa_new_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        tree_type* type,
        size_t reserved_operands,
        size_t size)
{
        ssa_instr* instr = ssa_allocate_node(context, size);
        if (!instr)
                return NULL;

        ssa_init_local_var(ssa_get_instr_var(instr), type);
        ssa_set_instr_kind(instr, kind);

        ssa_array* ops = &_ssa_instr_base(instr)->operands;
        ssa_init_array(ops);

        _ssa_init_instr_node(_ssa_instr_node(instr), NULL, false);
        return instr;
}

extern ssa_instr* ssa_new_unary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        tree_type* type,
        ssa_value* operand,
        size_t size)
{
        ssa_instr* instr = ssa_new_instr(context, kind, type, 1, size);
        if (!instr)
                return NULL;

        ssa_add_instr_operand(instr, context, operand);
        return instr;
}

extern ssa_instr* ssa_new_binary_instr(
        ssa_context* context,
        ssa_instr_kind kind,
        tree_type* type,
        ssa_value* first,
        ssa_value* second,
        size_t size)
{
        ssa_instr* instr = ssa_new_instr(context, kind, type, 2, size);
        if (!instr)
                return NULL;

        ssa_add_instr_operand(instr, context, first);
        ssa_add_instr_operand(instr, context, second);
        return instr;
}

static void ssa_init_value_use(ssa_value_use* self, ssa_instr* user)
{
        self->instr = user;
        self->value = NULL;
        list_node_init(&self->node);
}

extern ssa_value_use* ssa_add_instr_operand(ssa_instr* self, ssa_context* context, ssa_value* value)
{
        assert(value);
        ssa_array* ops = &_ssa_instr_base(self)->operands;
        ssa_array new_ops;
        ssa_init_array(&new_ops);
        ssa_resize_array(context, &new_ops, sizeof(ssa_value_use), ops->size + 1);
        for (size_t i = 0; i < ops->size; i++)
        {
                ssa_value_use* op = (ssa_value_use*)ops->data + i;
                ssa_value_use* new_op = (ssa_value_use*)new_ops.data + i;
                *new_op = *op;
                op->node.prev->next = &new_op->node;
                op->node.next->prev = &new_op->node;
        }
        ssa_dispose_array(context, ops);
        *ops = new_ops;
        
        ssa_value_use* last = (ssa_value_use*)ops->data + ops->size - 1;
        ssa_init_value_use(last, self);
        _ssa_add_value_use(value, last);
        return last;
}

extern bool ssa_instr_has_var(const ssa_instr* self)
{
        switch (ssa_get_instr_kind(self))
        {
                case SIK_STORE:
                case SIK_TERMINATOR:
                case SIK_FENCE:
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
        const ssa_array* ops = &_ssa_instr_cbase(self)->operands;
        assert(i < ops->size);
        return (ssa_value_use*)ops->data + i;
}

extern ssa_value_use* ssa_get_instr_operands_begin(const ssa_instr* self)
{
        return (ssa_value_use*)_ssa_instr_cbase(self)->operands.data;
}

extern ssa_value_use* ssa_get_instr_operands_end(const ssa_instr* self)
{
        return ssa_get_instr_operands_begin(self) + ssa_get_instr_operands_size(self);
}

extern size_t ssa_get_instr_operands_size(const ssa_instr* self)
{
        return _ssa_instr_cbase(self)->operands.size;
}

extern void ssa_add_instr_after(ssa_instr* self, ssa_instr* pos)
{
        _ssa_add_instr_node_after(_ssa_instr_node(self), _ssa_instr_node(pos));
}

extern void ssa_add_instr_before(ssa_instr* self, ssa_instr* pos)
{
        _ssa_add_instr_node_before(_ssa_instr_node(self), _ssa_instr_node(pos));
}

extern void ssa_remove_instr(ssa_instr* self)
{
        if (ssa_instr_has_var(self))
        {
                const ssa_value* var = ssa_get_instr_cvar(self);
                assert(!ssa_value_is_used(var) && "Cannot remove used instruction");
                assert(!ssa_get_value_metadata(var)
                        && "Delete metadata before removing instruction");
        }

        SSA_FOREACH_INSTR_OPERAND(self, it, end)
                _ssa_remove_value_use(it);

        _ssa_remove_instr_node(_ssa_instr_node(self));
        ssa_set_instr_kind(self, SIK_INVALID);
}

extern void ssa_move_instr(ssa_instr* self, ssa_instr* pos, bool after)
{
        struct _ssa_instr_node* node = _ssa_instr_node(self);
        _ssa_remove_instr_node(node);
        if (after)
                _ssa_add_instr_node_after(node, _ssa_instr_node(pos));
        else
                _ssa_add_instr_node_before(node, _ssa_instr_node(pos));
}

extern void ssa_set_instr_operand_value(ssa_instr* self, size_t i, ssa_value* val)
{
        ssa_set_value_use_value(ssa_get_instr_operand(self, i), val);
}

extern ssa_instr* ssa_new_alloca(ssa_context* context, tree_type* type)
{
        return ssa_new_instr(context, SIK_ALLOCA, type, 0, sizeof(struct _ssa_alloca));
}

extern tree_type* ssa_get_allocated_type(const ssa_instr* self)
{
        return tree_get_pointer_target(ssa_get_value_type(ssa_get_instr_cvar(self)));
}

extern ssa_instr* ssa_new_load(
        ssa_context* context, tree_type* type, ssa_value* what)
{
        return ssa_new_unary_instr(
                context, SIK_LOAD, type, what, sizeof(struct _ssa_load));
}

extern ssa_instr* ssa_new_cast(
        ssa_context* context, tree_type* type, ssa_value* operand)
{
        return ssa_new_unary_instr(
                context, SIK_CAST, type, operand, sizeof(struct _ssa_load));
}

extern ssa_instr* ssa_new_binop(
        ssa_context* context,
        tree_type* restype,
        ssa_binop_kind kind,
        ssa_value* lhs,
        ssa_value* rhs)
{
        ssa_instr* binop = ssa_new_binary_instr(
                context, SIK_BINARY, restype, lhs, rhs, sizeof(struct _ssa_binop));
        if (!binop)
                return NULL;

        ssa_set_binop_kind(binop, kind);
        return binop;
}

extern ssa_instr* ssa_new_store(ssa_context* context, ssa_value* what, ssa_value* where)
{
        return ssa_new_binary_instr(
                context, SIK_STORE, NULL, what, where, sizeof(struct _ssa_store));
}

extern ssa_instr* ssa_new_getfieldaddr(
        ssa_context* context,
        tree_type* type,
        ssa_value* operand,
        unsigned field_index)
{
        ssa_instr* gfa = ssa_new_unary_instr(
                context, SIK_GETFIELDADDR, type, operand, sizeof(struct _ssa_getfieldaddr));
        if (!gfa)
                return NULL;

        ssa_set_getfieldaddr_index(gfa, field_index);
        return gfa;
}

extern ssa_instr* ssa_new_call(ssa_context* context, tree_type* type, ssa_value* func)
{
        return ssa_new_unary_instr(context, SIK_CALL, type, func, sizeof(struct _ssa_call));
}

extern bool ssa_call_returns_void(const ssa_instr* self)
{
        ssa_value* func = ssa_get_called_func(self);
        tree_type* func_type = tree_desugar_type(
                tree_get_pointer_target(ssa_get_value_type(func)));
        return tree_type_is_void(tree_get_func_type_result(func_type));
}

extern ssa_instr* ssa_new_phi(ssa_context* context, tree_type* restype)
{
        return ssa_new_instr(context, SIK_PHI, restype, 0, sizeof(struct _ssa_phi));
}

extern void ssa_add_phi_operand(
        ssa_instr* self, ssa_context* context, ssa_value* value, ssa_value* label)
{
        ssa_add_instr_operand(self, context, value);
        ssa_add_instr_operand(self, context, label);
}

extern ssa_instr* ssa_new_terminator_instr(
        ssa_context* context,
        ssa_terminator_instr_kind kind,
        size_t reserved_operands,
        size_t size)
{
        ssa_instr* instr = ssa_new_instr(context,
                SIK_TERMINATOR, NULL, reserved_operands, size);
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
                        UNREACHABLE();
                        return NULL;
        }
}

extern ssa_value_use* ssa_get_next_terminator_successor(const ssa_instr* instr, ssa_value_use* pos)
{
        return ssa_get_terminator_instr_kind(instr) == STIK_SWITCH ? pos + 2 : pos + 1;
}

extern size_t ssa_get_terminator_instr_successors_size(const ssa_instr* self)
{
        size_t n = ssa_get_instr_operands_size(self);
        switch (ssa_get_terminator_instr_kind(self))
        {
                case STIK_INDERECT_JUMP:
                        return n;

                case STIK_CONDITIONAL_JUMP:
                case STIK_SWITCH:
                        return n - 1;

                case STIK_RETURN:
                        return 0;

                default:
                        UNREACHABLE();
                        return 0;
        }
}

extern ssa_instr* ssa_new_inderect_jump(ssa_context* context, ssa_value* dest)
{
        ssa_instr* jump = ssa_new_unary_instr(context, 
                SIK_TERMINATOR, NULL, dest, sizeof(struct _ssa_inderect_jump));
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

        ssa_add_instr_operand(jump, context, condition);
        ssa_add_instr_operand(jump, context, on_true);
        ssa_add_instr_operand(jump, context, on_false);
        return jump;
}

extern ssa_instr* ssa_new_switch_instr(ssa_context* context, ssa_value* condition)
{
        ssa_instr* jump = ssa_new_unary_instr(context,
                SIK_TERMINATOR, NULL, condition, sizeof(struct _ssa_switch_instr));
        if (!jump)
                return NULL;

        ssa_set_terminator_instr_kind(jump, STIK_SWITCH);
        return jump;
}

extern void ssa_add_switch_case(ssa_instr* self,
        ssa_context* context, ssa_value* value, ssa_value* label)
{
        ssa_add_instr_operand(self, context, value);
        ssa_add_instr_operand(self, context, label);
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

        ssa_add_instr_operand(ret, context, value);
        return ret;
}

extern ssa_instr* ssa_new_atomic_rmw_instr(
        ssa_context* context,
        tree_type* restype,
        ssa_atomic_rmw_instr_kind kind,
        ssa_value* ptr,
        ssa_value* val,
        ssa_memorder_kind ordering)
{
        ssa_instr* i = ssa_new_instr(context, SIK_ATOMIC_RMW, restype, 3,
                sizeof(struct _ssa_atomic_rmw_instr));
        if (!i)
                return NULL;

        ssa_set_atomic_rmw_instr_kind(i, kind);
        ssa_add_instr_operand(i, context, ptr);
        ssa_add_instr_operand(i, context, val);
        ssa_set_atomic_rmw_instr_ordering(i, ordering);
        return i;
}

extern ssa_instr* ssa_new_fence_instr(
        ssa_context* context, ssa_syncscope_kind syncscope, ssa_memorder_kind ordering)
{
        ssa_instr* i = ssa_new_instr(context, SIK_FENCE, NULL, 0, sizeof(struct _ssa_fence_instr));
        if (!i)
                return NULL;

        ssa_set_fence_instr_syncscope(i, syncscope);
        ssa_set_fence_instr_ordering(i, ordering);
        return i;
}

extern ssa_instr* ssa_new_atomic_cmpxchg_instr(
        ssa_context* context,
        tree_type* restype,
        ssa_value* ptr,
        ssa_value* expected,
        ssa_value* desired,
        ssa_memorder_kind success_ordering,
        ssa_memorder_kind failure_ordering)
{
        ssa_instr* i = ssa_new_instr(context, SIK_ATOMIC_CMPXCHG, restype, 3,
                sizeof(struct _ssa_atomic_cmpxchg_instr));
        if (!i)
                return NULL;

        ssa_add_instr_operand(i, context, ptr);
        ssa_add_instr_operand(i, context, expected);
        ssa_add_instr_operand(i, context, desired);
        ssa_set_atomic_cmpxchg_instr_success_ordering(i, success_ordering);
        ssa_set_atomic_cmpxchg_instr_failure_ordering(i, failure_ordering);
        return i;
}