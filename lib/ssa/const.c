#include "scc/ssa/const.h"
#include "scc/ssa/context.h"

extern ssa_const* ssa_new_const(
        ssa_context* context,
        ssa_const_kind kind,
        tree_type* type,
        size_t size)
{
        ssa_const* c = ssa_allocate_node(context, size);
        if (!c)
                return NULL;
        ssa_set_const_kind(c, kind);
        ssa_set_const_type(c, type);
        return c;
}

extern ssa_const* ssa_new_const_addr(
        ssa_context* context,
        tree_type* type,
        ssa_value* var)
{
        ssa_const* c = ssa_new_const(context, SCK_ADDRESS, type, sizeof(struct _ssa_const_addr));
        if (!c)
                return NULL;
        ssa_set_const_addr(c, var);
        return c;
}

extern ssa_const* ssa_new_const_expr(
        ssa_context* context,
        ssa_const_kind kind,
        tree_type* type,
        size_t num_operands)
{
        return ssa_new_const(context, kind, type, 
                sizeof(struct _ssa_const_expr) + sizeof(ssa_const*) * num_operands);
}

extern ssa_const* ssa_new_const_cast(
        ssa_context* context,
        tree_type* type,
        ssa_const* operand)
{
        ssa_const* c = ssa_new_const_expr(context, SCK_CAST, type, 1);
        if (!c)
                return NULL;
        ssa_set_const_expr_operand(c, 0, operand);
        return c;
}

extern ssa_const* ssa_new_const_ptradd(
        ssa_context* context,
        tree_type* type,
        ssa_const* ptr,
        ssa_const* offset)
{
        ssa_const* c = ssa_new_const_expr(context, SCK_PTRADD, type, 2);
        if (!c)
                return NULL;
        ssa_set_const_expr_operand(c, 0, ptr);
        ssa_set_const_expr_operand(c, 1, offset);
        return c;
}

extern ssa_const* ssa_new_const_literal(
        ssa_context* context,
        tree_type* type,
        struct num value)
{
        ssa_const* c = ssa_new_const(context, SCK_LITERAL, type, sizeof(struct _ssa_const_literal));
        if (!c)
                return NULL;
        ssa_set_const_literal(c, value);
        return c;
}

extern ssa_const* ssa_new_const_field_addr(
        ssa_context* context,
        tree_type* type,
        ssa_const* var,
        unsigned field_index)
{
        ssa_const* c = ssa_new_const(context, SCK_GETFIELDADDR, type, sizeof(struct _ssa_const_field_addr));
        if (!c)
                return NULL;
        ssa_set_const_field_addr_var(c, var);
        ssa_set_const_field_addr_index(c, field_index);
        return c;
}

extern ssa_const* ssa_new_const_list(
        ssa_context* context, tree_type* type)
{
        ssa_const* c = ssa_new_const(context, SCK_LIST, type, sizeof(struct _ssa_const_list));
        if (!c)
                return NULL;
        vec_init(ssa_get_const_list(c));
        return c;
}
